#include "Actor.h"
#include "Camera.h"
#include "MeshLoadHelper.h"
namespace hlab {
    Actor::Actor() {}
    Actor::Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const string& basePath, const string& filename)
    {
        Initialize(device, context, basePath, filename);
    }

    void Actor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const string& basePath, const string& filename)
    {
        InitBoundingKey();
        m_model = DModel(device, context, basePath, filename);
    }
    bool Actor::MsgProc(WPARAM wParam, shared_ptr<Actor> InActivateActore)
    {
        if (m_keyBinding.find(wParam) != m_keyBinding.end())
        {
            m_keyBinding[wParam]();
            return true;
        }
        return false;
    }
    //Camera ฐüทร
    void Actor::ActiveCaemera()
    {
        //m_camera->UpdatePosDir(m_cameraCorrection* m_actorConsts.GetCpu().world);
    }
    void Actor::UpdateCemeraCorrection(Vector3 deltaPos)
    {
        m_cameraCorrection = Matrix::CreateTranslation(deltaPos);
    }

    void Actor::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        m_model.Render(context);
    }


}