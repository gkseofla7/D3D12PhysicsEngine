#pragma once
#include "BillboardModel.h"
#include "DaerimsEngineBase.h"
namespace hlab {
class AppBase;
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
    void Initialize(AppBase* InBase, ComPtr<ID3D11Device> InDevice, ComPtr<ID3D11DeviceContext> InContext);
    btRigidBody* CreateProjectile(const Vector3& tV,
        float InRadius,
        const Vector3& Invelocity);
private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    AppBase* App;
};
}
