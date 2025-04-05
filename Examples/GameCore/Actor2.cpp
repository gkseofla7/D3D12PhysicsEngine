#include "Actor2.h"
#include "Legacy/Camera.h"
#include "MeshLoadHelper2.h"
#include "GameDef2.h"
#include "ActorStateFactory2.h" 
#include "ActorState2.h"
  
//#include "BulletDynamics/btBulletDynamicsCommon.h"
//#include "DaerimsEngineBase.h"
namespace dengine {
Actor::Actor() {}
Actor::Actor(shared_ptr<DModel> inModel)
{
    Initialize(inModel);
    m_model = inModel;
}
void Actor::Initialize(shared_ptr<DModel> inModel)
{
    Object::Initialize(inModel);
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
    Object::Tick(dt);
    // TODO. physics ���� �ӽ� �ּ� ó��
    /*
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
     */

    Vector3 deltaPos = m_velocity * Vector3(0., 0., -dt);
    if (m_externalForce > 0.0f)
    {
        // TODO. ���� Model������ ���⿡���� Delta���� �ް��־� ������ �ݴ�������� �̵��ϵ��� �ص�
        // ���߿� �� �������� ���󰡵��� ���� ����(���� �ݴ�������� ���͸� ȸ�����ѵױ⶧���� ������ ����)
        deltaPos += m_externalForce*Vector3(0., 0., dt);
        //m_externalForce = m_externalForce - m_externalForce / m_externalForce.Length() * 0.1f;
    }
    UpdatePosition(deltaPos);

    UpdateState();
}

void Actor::RequestStateChange(EActorStateType InType)
{
    SetState(InType);
}

//Camera ����
void Actor::ActiveCamera()
{
    //m_camera->UpdatePosDir(m_cameraCorrection* m_actorConsts.GetCpu().world);
}
void Actor::UpdateCameraCorrection(Vector3 deltaPos)
{
    m_cameraCorrection = Matrix::CreateTranslation(deltaPos);
}

void Actor::Render()
{
    m_model->Render();
}
// ���� ����
void Actor::SetState(EActorStateType InType)
{
    m_prevStateType = m_actorStateType;
    m_actorStateType = InType;
}
void Actor::UpdateState()
{
    if (m_actorState.get() == nullptr || m_actorState.get()->GetStateType() != m_actorStateType)
    {
        m_actorState = ActorStateFactory::GetInstance().CreateActorState(m_actorStateType, shared_from_this());
        m_actorState->Initialize();
    }
}
}