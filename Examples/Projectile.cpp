#include "Projectile.h"
#include "Legacy/BillboardModel.h"
#include "Legacy/DModel.h"
#include "Legacy/GameUtility.h"

#include "Legacy/DaerimsEngineBase.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "bullet/btBulletDynamicsCommon.h" 

namespace hlab {
    bool IsObjectInContact(btCollisionObject* obj, btCollisionWorld* world, btCollisionObject* & OutCollisionObject) {
        class SimpleContactCallback : public btCollisionWorld::ContactResultCallback {
        public:
            bool hasCollided = false;
            btCollisionObject* collisionObj = nullptr;
            btScalar addSingleResult(btManifoldPoint& cp,
                const btCollisionObjectWrapper* colObj0Wrap,
                int partId0,
                int index0,
                const btCollisionObjectWrapper* colObj1Wrap,
                int partId1,
                int index1) override {
                hasCollided = true;  // 충돌이 발생했음을 기록
                collisionObj = const_cast<btCollisionObject*>(const_cast<btCollisionObjectWrapper*>(colObj1Wrap)->getCollisionObject());
                return 0;  // 기본 반환값
            }
        };

        SimpleContactCallback callback;
        world->contactTest(obj, callback);  // 충돌 테스트 수행
        OutCollisionObject = callback.collisionObj;
        return callback.hasCollided;  // 충돌 여부 반환
    }
    void Projectile::Initialize(ComPtr<ID3D11Device>& InDevice, ComPtr<ID3D11DeviceContext>& InContext,
        const Vector3& InPos, const Vector3& InVelocity, float InRadius)
    {
        m_model =  std::make_shared<BillboardModel>();
        static const float simToRenderScale = 0.01f;
        m_projectileVelocity = InVelocity* simToRenderScale;
        Vector3 dir(float(InVelocity.x), float(InVelocity.y), float(InVelocity.z));
        dir.Normalize();
        shared_ptr<BillboardModel> billboardModel =std::dynamic_pointer_cast<BillboardModel>(m_model);
        billboardModel->m_billboardConsts.m_cpu.directionWorld = dir;
        billboardModel->m_castShadow = false;
        billboardModel->Initialize(InDevice, InContext, { {0.0f, 0.0f, 0.0f, 1.0f} },
            0.2f, Graphics::volumetricFirePS);
        billboardModel->SetWorldPosition(InPos);
        m_usePhysicsSimulation = true;
    }

    void Projectile::Tick(float dt)
    {
        Object::Tick(dt);
        static const float simToRenderScale = 0.01f;
        // TODO. 쏜 사람에 대해서는 예외처리하는게 더 낫지 않을지..
        m_elasedSeconds += dt;
        static const float notCollapsedTime = 0.3f;
        if (m_elasedSeconds > notCollapsedTime)
        {
            if (m_physicsBody != nullptr)
            {
                // TODO. 이것도 이동 필요
                if (m_startPendingKill == false)
                {
                    btCollisionObject* collisionObj = (btCollisionObject*)(m_physicsBody);
                    btCollisionObject* hittedCollisionObject = nullptr;
                    if (IsObjectInContact(collisionObj, DaerimsEngineBase::GetInstance().GetDynamicWorld(), hittedCollisionObject))
                    {
                        m_startPendingKill = true;
                        if (hittedCollisionObject != nullptr)
                        {
                            std::weak_ptr<Object>  projectileObj = DaerimsEngineBase::GetInstance().GetPhysObject(hittedCollisionObject);
                            std::shared_ptr<Object> projectileObjLock = projectileObj.lock();
                            if (projectileObjLock.get() != nullptr)
                            {
                                // 부딪힌 후에 반작용의 속도를 사용중이라.. 이것도 수정 필요. 딱 부딪힐때 속도를 전달해야,,
                                //Vector3 myVel = TransfromVector(m_physicsBody->getLinearVelocity())* simToRenderScale;
                                float momentum = m_physicsBody->getMass() *(m_projectileVelocity.Length())* m_projectileVelocity.Length()/2.0f;
                                projectileObjLock->AddMomentum(momentum , m_projectileVelocity);
                                projectileObjLock->ReactProjectileHitted();
                            }
                        }
                    }
                }
                else
                {
                    m_pendingKillElapsedSeconds += dt;
                    static const float killTime = 0.1f;
                    if (m_pendingKillElapsedSeconds > killTime)
                    {
                        SetPendingKill(true);
                    }
                }
            }
        }
    }
}