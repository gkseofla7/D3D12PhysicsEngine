#include "Actor.h"
#include "Camera.h"
#include "MeshLoadHelper.h"
#include "ActorStateFactory.h" 
#include "ActorState.h"
  
#include "BulletDynamics/btBulletDynamicsCommon.h"
#include "DaerimsEngineBase.h"
namespace hlab {
    Actor::Actor() {}
    Actor::Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> inModel)
    {
        Initialize(device, context, inModel);
        m_model = inModel;
    }
    void Actor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> inModel)
    {
        Object::Initialize(device, context, inModel);
        SetState(EActorStateType::Idle);
       
    }
    bool Actor::MsgProc(WPARAM wParam, bool bPress)
    {
        if (bPress)
        {
            if (m_keyBindingPress.find(wParam) != m_keyBindingPress.end())
            {
                m_keyBindingPress[wParam]();
                return true;
            }
        }
        else
        {
            if (m_keyBindingRelease.find(wParam) != m_keyBindingRelease.end())
            {
                m_keyBindingRelease[wParam]();
                return true;
            }
        }

        return false;
    }
    void Actor::Tick(float dt)
    {
        Object::Tick( dt);
        if (m_physicsBody == nullptr && m_model->IsMeshInitialized())
        {
            Vector3 worldPos = GetWorldPosition();
            static const float simToRenderScale = 0.01f;
            btTransform t = btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(worldPos.x, worldPos.y, worldPos.z) /
                simToRenderScale);

            btVector3 cylinderExtent = btVector3(m_model->m_boundingBox.Extents.x/ simToRenderScale,
                m_model->m_boundingBox.Extents.y/ simToRenderScale, m_model->m_boundingBox.Extents.z/ simToRenderScale);
            btCylinderShape* cylinderShape = new btCylinderShape(cylinderExtent);
            btRigidBody* dynamic =
                DaerimsEngineBase::GetInstance().CreateRigidBody(5.0, t, cylinderShape, 0.5f, btVector4(0, 0, 1, 1));
            DaerimsEngineBase::GetInstance().RegisterPhysMap(dynamic, shared_from_this());
            m_physicsBody = dynamic;
        }


        Vector3 deltaPos = m_velocity * Vector3(0., 0., -dt);
        if (m_externalForce > 0.0f) 
        {
            // TODO. 현재 Model에대한 방향에대한 Delta값을 받고있어 액터의 반대방향으로 이동하도록 해둠
            // 나중에 힘 방향으로 날라가도록 수정 예정(힘의 반대방향으로 액터를 회전시켜뒀기때문에 지금은 괜찮)
            deltaPos += m_externalForce*Vector3(0., 0., dt);
            //m_externalForce = m_externalForce - m_externalForce / m_externalForce.Length() * 0.1f;
        }
        UpdatePosition(deltaPos);
        UpdateState();

    }

    //Camera 관련
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
    // 상태 관련
    void Actor::SetState(EActorStateType InType)
    {
        m_prevStateType = m_actorStateType;
        m_actorStateType = InType;
    }
    void Actor::UpdateState()
    {
        if (m_actorState.get() == nullptr ||m_actorState.get()->GetStateType() != m_actorStateType)
        {
            m_actorState = ActorStateFactory::GetInstance().CreateActorState(m_actorStateType, shared_from_this());
            m_actorState->Initialize();
        }
    }
}