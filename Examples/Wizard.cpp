#include "Wizard.h"
#include "AnimHelper.h"
#include "ActorStateFactory.h"
#include "ActorState.h"
#include "ProjectileManager.h"
#include "magic_enum.hpp"

namespace hlab {

Wizard::Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	shared_ptr<DModel> InModel)
    :SkeletalMeshActor(device, context, InModel)
{
	//Initialize(device, context, InModel);
}
void Wizard::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	shared_ptr<DModel> InModel)
{
    SkeletalMeshActor::Initialize(device, context,InModel);
	InitBoundingKey();
    // RandomNumber 받도록 한다.
    m_model->m_modelId = 1;

    
    // 애니메이션 관련, 따로 테이블을 만들어서 관리하는게..
    string path = "../Assets/Characters/Mixamo/";
    AnimHelper::GetInstance().AddAnimPath(m_model->m_modelId, path);
    // Idle
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(ActorStateType::Idle).data(), "FightingIdleOnMichelle2.fbx");
    // Attack
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(ActorStateType::Attack).data(), "Fireball.fbx");
    // Move
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateIdleToWalk).data(), "Female Start Walking.fbx");
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateWalk).data(), "Walking.fbx");
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateWalkToIdle).data(), "Female Stop Walking.fbx");
}
 
void Wizard::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
{
    SkeletalMeshActor::Update(device, context, dt);
    static const float simToRenderScale = 0.01f;
    m_actorState->Tick(dt);

    if (m_actorState->GetStateType() == ActorStateType::Attack)
    {
        static const int FIreBallStartFrame = 115;
        if (m_actorState->GetFrame() == FIreBallStartFrame) {
            Vector3 handPos = (GetModel()->m_worldRow).Translation();
            Vector4 offset = Vector4::Transform(
                Vector4(0.0f, 0.0f, -0.1f, 0.0f),
                GetModel()->m_worldRow *
                accumulatedRootTransform);
            handPos += Vector3(offset.x, offset.y, offset.z);

            Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
            dir = Vector4::Transform(
                dir, GetModel()->m_worldRow *
                accumulatedRootTransform);
            dir.Normalize();
            dir *= 1.5f / simToRenderScale;
            // 직접 만드는것보단 요청하는게..
            ProjectileManager::GetInstance().CreateProjectile(handPos,
                5, Vector3(dir.x, dir.y, dir.z));
        }
    }
    m_curFrame += 1;
}
void Wizard::InitBoundingKey()
{
	std::function<void()> ShotFireballFunc = [this]() { this->ShotFireball();};
    m_keyBindingPress.insert({ VK_SPACE, ShotFireballFunc });
    // 걷기 관련
    std::function<void()> WalkPressFunc = [this]() { this->WalkStart(); };
    m_keyBindingPress.insert({ VK_UP, WalkPressFunc });
    std::function<void()> WalkReleaseFunc = [this]() { this->WalkEnd(); };
    m_keyBindingRelease.insert({ VK_UP, WalkReleaseFunc });
}

void Wizard::ShotFireball()
{
    if (m_actorStateType == ActorStateType::Idle)
    {
        SetState(ActorStateType::Attack);
    }	
}
void Wizard::WalkStart()
{
    if (m_actorState->GetStateType() == ActorStateType::Idle)
    {
        SetState(ActorStateType::Move);
    }
}
void Wizard::WalkEnd()
{
    if (m_actorState->GetStateType() == ActorStateType::Move)
    {// TODO 이거 수정 필요
        m_actorState->Transition();
    }
}
}