#pragma once

#include "Legacy/Model.h"
#include "Legacy/Object.h"
#include <memory>
class btRigidBody;
namespace hlab {
    
    class Projectile : public Object
    {
    public:
        void Initialize(ComPtr<ID3D11Device>& InDevice, ComPtr<ID3D11DeviceContext>& InContext,
            const Vector3& InPos, const Vector3& InVelocity, float InRadius);
        void Tick(float dt);
    private:
        float m_elasedSeconds = 0.0f;

        bool m_startPendingKill = false;
        float m_pendingKillElapsedSeconds = 0.0;

        // TODO. Object Velocity랑 중복된다, 수정이 필요하다.
        Vector3 m_projectileVelocity;
    };
}
