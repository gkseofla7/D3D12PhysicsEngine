#include "DaerimGame.h"
#include "MeshLoadHelper2.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletCollision/btBulletCollisionCommon.h"
#include "Object2.h"
namespace dengine {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

DaerimGame::DaerimGame() :Engine() {}

bool DaerimGame::InitScene()
{
    Engine::InitScene();
    InitPhysics(true);

    return true;
}
void DaerimGame::InitPhysics(bool interactive)
{
    PhysicsCore::GetInstance().InitPhysEngine();
    //m_dynamicsWorld->setGravity(btVector3(0,0,0));

    ///create a few basic rigid bodies
    btStaticPlaneShape* groundShape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), btScalar(1.0f));

    //groundShape->initializePolyhedralFeatures();
    //btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);

    //m_collisionShapes.push_back(groundShape);


    {
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, 0, 0));
        btScalar mass(0.);
        PhysicsCore::GetInstance().CreateRigidBody(mass, groundTransform, groundShape, 0.0f, btVector4(0, 0, 1, 1));
    }

    for (unsigned int i = 0; i < 1; i++)
    {
        btTransform t = btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0, 0, stackZ -= 15.0f));
        CreateStack(t, 8, 20, 2.5f);
    }
}

void DaerimGame::Update(float dt) {
    Engine::Update(dt);

    // 물리엔진 관련
    StepSimulation(dt);

    btDiscreteDynamicsWorld* DynamicWorld = PhysicsCore::GetInstance().GetDynamicWorld();
    int count = 0;
    int numCollisionObjects = DynamicWorld->getNumCollisionObjects();
    {
        for (int i = 0; i < numCollisionObjects; i++)
        {
            btCollisionObject* colObj = DynamicWorld->getCollisionObjectArray()[i];
            if (colObj == nullptr)
            {
                continue;
            }

            if (!colObj->isStaticObject())
            {
                std::weak_ptr<Object> obj = PhysicsCore::GetInstance().GetPhysObject(colObj);
                if (obj.expired())
                {
                    PhysicsCore::GetInstance().RemovePhysMap(colObj);
                    continue;
                }
                std::shared_ptr<Object> objectLock = obj.lock();
                if (objectLock->IsUsePhsycsSimulation() == false)
                {
                    continue;
                }
                // TODO count 인덱스로 찾는게 아닌 map으로 검색하도록 수정
                btCollisionShape* collisionShape = colObj->getCollisionShape();
                btVector3 pos = colObj->getWorldTransform().getOrigin();
                btQuaternion orn = colObj->getWorldTransform().getRotation();
                //Matrix::CreateFromQuaternion(orn.get128())*
                //    Matrix::CreateTranslation(pos.get128())*
                objectLock->UpdateWorldRow(Matrix::CreateTranslation(pos.get128()) *
                    Matrix::CreateScale(m_simToRenderScale)); // PhysX to Render 스케일
                //m_objectList[count]->UpdateConstantBuffers(m_device, m_context);
                count++;
            }
        }
    }
}


void DaerimGame::CreateStack(const btTransform t, int numStacks,
    int numWidth, btScalar halfExtent) {
    //vector<MeshData> box = { GeometryGenerator::MakeBox(halfExtent) };
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -50, 0));

    //	//create a few dynamic rigidbodies
//	// Re-using the same collision is better for memory usage and performance

    btBoxShape* colShape = PhysicsCore::GetInstance().CreateBoxShape(btVector3(halfExtent, halfExtent, halfExtent));


    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar mass(1.f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass, localInertia);
    string meshKey = MeshLoadHelper::LoadBoxMesh(halfExtent);
    for (int i = 0; i < numStacks; i++)
    {
        for (int j = 0; j < numWidth - i; j++)
        {
            btTransform localTm;
            localTm.setIdentity();
            localTm.setOrigin(btVector3(btScalar(j * 2) - btScalar(numWidth - i),
                btScalar(i * 2 + 1) + 5.0, 0.0) *
                halfExtent);
            localTm.setBasis(btMatrix3x3::getIdentity());
            btRigidBody* body = PhysicsCore::GetInstance().CreateRigidBody(mass, t * localTm, colShape, 0.0f, btVector4(0, 0, 1, 1));

            
            MaterialConstants materialConsts = MaterialConstants();
            materialConsts.albedoFactor = Vector3(0.8f);

            auto newModel = std::make_shared<DModel>(meshKey);
            newModel->SetMaterialConstants(materialConsts);

            auto newObj = std::make_shared<Object>();
            newObj->Initialize(newModel);
            newObj->SetPhysicsBody(body);

            PhysicsCore::GetInstance().RegisterPhysMap(body, newObj);
            m_objectList.push_back(newObj);
            newObj->SetUsePhsycisSimulation(true);
        }
    }
}

void DaerimGame::StepSimulation(float deltaTime)
{
    PhysicsCore::GetInstance().StepSimulation(deltaTime);
}
}

//btRigidBody* DaerimGame::CreateDynamic(const btTransform& t,
//    btCollisionShape* shape,
//    const btVector3& velocity) {
//    m_fireball = std::make_shared<BillboardModel>();
//    //m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
//    //                       1.0f, L"GameExplosionPS.hlsl");
//    Vector3 dir(float(velocity.getX()), float(velocity.getY()), float(velocity.getZ()));
//    dir.Normalize();
//    m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
//    m_fireball->m_castShadow = false;
//    m_fireball->Initialize(m_device, m_context, { {0.0f, 0.0f, 0.0f, 1.0f} },
//        0.2f, Graphics::volumetricFirePS);
//
//    AppBase::m_basicList.push_back(m_fireball);
//    m_objects.push_back(m_fireball);
//
//    btRigidBody* dynamic = 
//        PhysicsCore::CreateRigidBody(m_dynamicsWorld,5.0, t, shape, 0.5f, btVector4(0, 0, 1, 1));
//    dynamic->setLinearVelocity(velocity);
//
//    return dynamic;
//}