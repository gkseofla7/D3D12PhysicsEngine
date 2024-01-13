#include "Ex1901_PhysX.h"

#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "OceanModel.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1901_PhysX::Ex1901_PhysX() : AppBase() {}

void Ex1901_PhysX::InitPhysics(bool interactive) {
    gFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport *transport =
        PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
                               PxTolerancesScale(), true, gPvd);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
                                   true);
    }
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic *groundPlane =
        PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);

    for (PxU32 i = 0; i < 5; i++)
        createStack(PxTransform(PxVec3(0, 1.0f, stackZ -= 1.0f)), 15, 0.2f);

    if (!interactive)
        createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10),
                      PxVec3(0, -50, -100));
}

bool Ex1901_PhysX::InitScene() {

    AppBase::m_camera.Reset(Vector3(-11.9666f, 4.85741f, -1.85711f), 0.981748f,
                            0.239983f);

    AppBase::m_globalConstsCPU.strengthIBL = 1.0f;
    AppBase::m_globalConstsCPU.lodBias = 0.0f;

    //AppBase::InitCubemaps(
    //    L"../Assets/Textures/Cubemaps/HDRI/", L"clear_pureskyEnvHDR.dds",
    //    L"clear_pureskySpecularHDR.dds", L"clear_pureskyDiffuseHDR.dds",
    //    L"clear_pureskyBrdf.dds");
     
    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");
    AppBase::InitScene();

    InitPhysics(true);

    return true;
}

void Ex1901_PhysX::UpdateLights(float dt) { AppBase::UpdateLights(dt); }

void Ex1901_PhysX::Update(float dt) {

    AppBase::Update(dt);

    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);

    // gScene->getActors()
    // PxGeometryType::eBOX: , case PxGeometryType::eSPHERE:

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                         PxActorTypeFlag::eRIGID_STATIC);
    std::vector<PxRigidActor *> actors(nbActors);
    gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                          PxActorTypeFlag::eRIGID_STATIC,
                      reinterpret_cast<PxActor **>(&actors[0]), nbActors);

    PxShape *shapes[MAX_NUM_ACTOR_SHAPES];

    int count = 0;

    for (PxU32 i = 0; i < nbActors; i++) {
         
        const PxU32 nbShapes = actors[i]->getNbShapes();
        PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
        actors[i]->getShapes(shapes, nbShapes);
        for (PxU32 j = 0; j < nbShapes; j++) {
            const PxMat44 shapePose(
                PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

            if (actors[i]->is<PxRigidDynamic>()) {

                bool speeping = actors[i]->is<PxRigidDynamic>() &&
                                actors[i]->is<PxRigidDynamic>()->isSleeping();
                // cout << i << " : " << shapePose.getPosition().y << " sleeping
                // "
                //      << speeping << endl;

                m_objects[count]->UpdateWorldRow(Matrix(shapePose.front()) *
                                                 Matrix::CreateScale(1.00f));
                m_objects[count]->UpdateConstantBuffers(m_device, m_context);

                count++;
            }
        }
    }

    /* PxContactPair 추출 하면 내 캐릭터가 어디에 닿았는지 찾을 수 있음
    * void onContact(const PxContactPairHeader& pairHeader, const PxContactPair*
    pairs, PxU32 nbPairs)
    {
        PX_UNUSED((pairHeader));
        std::vector<PxContactPairPoint> contactPoints;

        for(PxU32 i=0;i<nbPairs;i++)
        {
            PxU32 contactCount = pairs[i].contactCount;
            if(contactCount)
            {
                contactPoints.resize(contactCount);
                pairs[i].extractContacts(&contactPoints[0], contactCount);

                for(PxU32 j=0;j<contactCount;j++)
                {
                    gContactPositions.push_back(contactPoints[j].position);
                    gContactImpulses.push_back(contactPoints[j].impulse);
                }
            }
        }
    }
    */
}

void Ex1901_PhysX::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void Ex1901_PhysX::UpdateGUI() { AppBase::UpdateGUI(); }

} // namespace hlab
