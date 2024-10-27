#include "ProjectileManager.h"
#include "AppBase.h"
#include "Projectile.h"

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
    
    btTransform t = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(tV.x, tV.y, tV.z) /
        simToRenderScale);
    btVector3 velocity = btVector3(Invelocity.x, Invelocity.y, Invelocity.z);
    btSphereShape* SphereShape = new btSphereShape(InRadius * 0.5);
    

    //shared_ptr<BillboardModel> m_fireball = std::make_shared<BillboardModel>();
    ////m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
    ////                       1.0f, L"GameExplosionPS.hlsl");
    //Vector3 dir(float(Invelocity.x), float(Invelocity.y), float(Invelocity.z));
    //dir.Normalize();
    //m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
    //m_fireball->m_castShadow = false;
    //m_fireball->Initialize(m_device, m_context, { {0.0f, 0.0f, 0.0f, 1.0f} },
    //    0.2f, Graphics::volumetricFirePS);
    //m_fireball->m_isCollision = true;

    shared_ptr<Projectile> projectile = std::make_shared<Projectile>();
    projectile->Initialize(m_device, m_context, tV, Invelocity, InRadius);

    btRigidBody* dynamic =
        DaerimsEngineBase::GetInstance().CreateRigidBody( 5.0, t, SphereShape, 0.5f, btVector4(0, 0, 1, 1));
    dynamic->setLinearVelocity(velocity);
    dynamic->clearGravity();
    projectile->SetPhysicsBody(dynamic);
    App->m_objectList.push_back(projectile);
    //m_physList[] = (newObj);
    DaerimsEngineBase::GetInstance().RegisterPhysMap(dynamic, projectile);

    return dynamic;
}
}