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
        shared_ptr<DModel> InModel)
    {
        Initialize(device, context, InModel);
        m_model = InModel;
    }
    void Actor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
    {
        Object::Initialize(device, context, InModel);
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
            Vector3 WorldPos = GetWorldPosition();
            static const float simToRenderScale = 0.01f;
            btTransform t = btTransform(btQuaternion(), btVector3(WorldPos.x, WorldPos.y, WorldPos.z) /
                simToRenderScale);

            btVector3 CylinderExtent = btVector3(m_model->m_boundingBox.Extents.x/ simToRenderScale,
                m_model->m_boundingBox.Extents.y/ simToRenderScale, m_model->m_boundingBox.Extents.z/ simToRenderScale);
            btCylinderShape* CylinderShape = new btCylinderShape(CylinderExtent);
            btRigidBody* dynamic =
                DaerimsEngineBase::GetInstance().CreateRigidBody(5.0, t, CylinderShape, 0.5f, btVector4(0, 0, 1, 1));
            DaerimsEngineBase::GetInstance().RegisterPhysMap(dynamic, shared_from_this());
            m_physicsBody = dynamic;
        }

        Vector3 DeltaPos = m_velocity * Vector3(0., 0., -dt);
        UpdatePosition(DeltaPos);
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