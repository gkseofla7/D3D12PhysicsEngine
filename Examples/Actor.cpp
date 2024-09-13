#include "Actor.h"
#include "Camera.h"
#include "MeshLoadHelper.h"
#include "ActorStateFactory.h"
namespace hlab {
    Actor::Actor() {}
    Actor::Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
        :m_model(InModel)
    {
        m_model = InModel;
    }
    void Actor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
    {
        SetState(ActorStateType::Idle);
    }
    bool Actor::MsgProc(WPARAM wParam)
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
        m_model->Render(context);
    }
    void Actor::SetState(ActorStateType InType)
    {
        m_actorState = ActorStateFactory::GetInstance().CreateActorState(InType, shared_from_this());
    }

}