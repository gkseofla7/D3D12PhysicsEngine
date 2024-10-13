#pragma once

#include "Model.h"
#include "Object.h"
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
    };
}
