#include "ProjectileManager.h"
#include "AppBase.h"

#include "DaerimsEngineBase.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletDynamics/Dynamics/btRigidBody.h"
#include "bullet/BulletDynamics/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/btBulletCollisionCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorldImporter.h"
namespace hlab {
void ProjectileManager::Initialize(AppBase* InBase, ComPtr<ID3D11Device> InDevice, ComPtr<ID3D11DeviceContext> InContext)
{
    App = InBase; 
    m_device = InDevice;
    m_context = InContext;
}
btRigidBody* ProjectileManager::CreateProjectile(const Vector3& tV,
        float InRadius,
        const Vector3& Invelocity)
{
    static const float simToRenderScale = 0.01f;
    
    btTransform t = btTransform(btQuaternion(), btVector3(tV.x, tV.y, tV.z) /
        simToRenderScale);
    btSphereShape* SphereShape = new btSphereShape(InRadius);
       // btVector3(dir.x, dir.y, dir.z));
    btVector3 velocity = btVector3(Invelocity.x, Invelocity.y, Invelocity.z);

    shared_ptr<BillboardModel> m_fireball = std::make_shared<BillboardModel>();
    //m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
    //                       1.0f, L"GameExplosionPS.hlsl");
    Vector3 dir(float(Invelocity.x), float(Invelocity.y), float(Invelocity.z));
    dir.Normalize();
    m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
    m_fireball->m_castShadow = false;
    m_fireball->Initialize(m_device, m_context, { {0.0f, 0.0f, 0.0f, 1.0f} },
        0.2f, Graphics::volumetricFirePS);

    App->m_billboardModelList.push_back(m_fireball);
    App->m_objects.push_back(m_fireball);

    btRigidBody* dynamic =
        DaerimsEngineBase::CreateRigidBody(App->m_dynamicsWorld, 5.0, t, SphereShape, 0.5f, btVector4(0, 0, 1, 1));
    dynamic->setLinearVelocity(velocity);

    return dynamic;
}
}