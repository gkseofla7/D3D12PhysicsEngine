#pragma once
#include "D3D12Core/EnginePch.h"
#include "Legacy/BillboardModel.h"
#include "Legacy/DaerimsEngineBase.h"
namespace dengine {
class ProjectileManager
{
private:
    ProjectileManager() {}
public:
    static ProjectileManager& GetInstance()
    {
        static ProjectileManager Manager;
        return Manager;
    }
    btRigidBody* CreateProjectile(const Vector3& tV,
        float InRadius,
        const Vector3& Invelocity);
};
}
