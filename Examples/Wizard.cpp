#include "Wizard.h"
#include "AnimHelper.h"
#include "ActorStateFactory.h"
#include "ActorState.h"
#include "ProjectileManager.h"
#include "DSkinnedMeshModel.h"
#include "MoveState.h"
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
    // RandomNumber 받도록 한다.
    m_model->m_modelId = 1;

	InitBoundingKey();
    InitAnimPath();
}
 
void Wizard::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
{
    SkeletalMeshActor::Update(device, context, dt);

    m_actorState->Tick(dt);

    if (m_actorState->GetStateType() == ActorStateType::Attack)
    {
        static const int FIreBallStartFrame = 115;
        if (m_actorState->GetFrame() == FIreBallStartFrame) {
            ShotFireBall();
        }
    }
    m_curFrame += 1;
}

void Wizard::InitAnimPath()
{
    // 애니메이션 관련, 따로 테이블을 만들어서 관리하는게..
    string path = "../Assets/Characters/Mixamo/";
    AnimHelper::GetInstance().AddAnimPath(m_model->m_modelId, path);
    // Idle
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(ActorStateType::Idle).data(), "FightingIdleOnMichelle2.fbx");
    //LoadAnimAsync(magic_enum::enum_name(ActorStateType::Idle).data());
    // Attack
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(ActorStateType::Attack).data(), "Fireball.fbx");
    //LoadAnimAsync(magic_enum::enum_name(ActorStateType::Attack).data());
    // Move
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateIdleToWalk).data(), "Female Start Walking.fbx");
    //LoadAnimAsync(magic_enum::enum_name(MoveStateType::MoveStateIdleToWalk).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateWalk).data(), "Walking.fbx");
    //LoadAnimAsync(magic_enum::enum_name(MoveStateType::MoveStateWalk).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(MoveStateType::MoveStateWalkToIdle).data(), "Female Stop Walking.fbx");
    //LoadAnimAsync(magic_enum::enum_name(MoveStateType::MoveStateWalkToIdle).data());
    // Jumping
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(JumpStateType::JumpStateInPlace).data(), "Jumping.fbx");
    //LoadAnimAsync(magic_enum::enum_name(JumpStateType::JumpStateInPlace).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(JumpStateType::JumpStateRunning).data(), "Jump.fbx");
    //LoadAnimAsync(magic_enum::enum_name(JumpStateType::JumpStateRunning).data());
    
    // 쓰러짐
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->m_modelId, magic_enum::enum_name(ActorStateType::FlyAway).data(), "Stunned.fbx");
    //LoadAnimAsync(magic_enum::enum_name(ActorStateType::FlyAway).data());
}

void Wizard::LoadAnimAsync(string InState)
{

}

void Wizard::InitBoundingKey()
{
    // TODO. 인풋을.. State에서 받는게 나을듯 싶은데
	std::function<void()> ShotFireballFunc = [this]() {
        if (m_actorState->ActionKeyIfBind(VK_SPACE, true) == false)
        {
            this->Attack();
        }
    };
    m_keyBindingPress.insert({ VK_SPACE, ShotFireballFunc });
    // 걷기 관련
    std::function<void()> WalkPressFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_UP, true) == false)
        {
            this->WalkStart();
        }
    };
    m_keyBindingPress.insert({ VK_UP, WalkPressFunc });

    std::function<void()> WalkReleaseFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_UP, false) == false)
        {
            this->WalkEnd();
        }
    };
    m_keyBindingRelease.insert({ VK_UP, WalkReleaseFunc });

    std::function<void()> LeftPressFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_LEFT, true) == false)
        {
            this->RotateLeft(true);
        }
    };
    m_keyBindingPress.insert({ VK_LEFT, LeftPressFunc });

    std::function<void()> LeftReleaseFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_LEFT, false) == false)
        {
            this->RotateLeft(false);
        }
    };
    m_keyBindingRelease.insert({ VK_LEFT, LeftReleaseFunc });

    std::function<void()> RightPressFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_LEFT, true) == false)
        {
            this->RotateRight(true);
        }
    };
    m_keyBindingPress.insert({ VK_RIGHT, RightPressFunc });

    std::function<void()> RightReleaseFunc = [this]() { 
        if (m_actorState->ActionKeyIfBind(VK_RIGHT, false) == false)
        {
            this->RotateRight(false);
        }
    };
    m_keyBindingRelease.insert({ VK_RIGHT, RightReleaseFunc });

    std::function<void()> JumpFunc = [this]() {
        if (m_actorState->ActionKeyIfBind(VK_SHIFT, true) == false)
        {
            this->Jump();
        }
    };
    m_keyBindingPress.insert({ VK_SHIFT, JumpFunc });
}

// 인풋 관련 함수들
void Wizard::Attack()
{
    if (m_actorState->GetStateType() == ActorStateType::Idle)
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
    {
        m_actorState->Finish();
    }
    else if (m_prevStateType == ActorStateType::Move)
    {
        m_prevStateType = ActorStateType::Idle;
    }
}

void Wizard::RotateLeft(bool InOn)
{
    if (m_actorState->GetStateType() == ActorStateType::Move)
    {
        std::shared_ptr<MoveState> derivedPtr = std::dynamic_pointer_cast<MoveState>(m_actorState);
        derivedPtr->RotateLeft(InOn);
    }
}
void Wizard::RotateRight(bool InOn)
{
    if (m_actorState->GetStateType() == ActorStateType::Move)
    {
        std::shared_ptr<MoveState> derivedPtr = std::dynamic_pointer_cast<MoveState>(m_actorState);
        derivedPtr->RotateRight(InOn);
    }
}

void Wizard::Jump()
{
    SetState(ActorStateType::Jump);
}

void Wizard::ShotFireBall()
{
    static const float simToRenderScale = 0.01f;

    Vector4 offset = Vector4::Transform(
        Vector4(0.0f, 0.0f, -0.1f, 1.0f), GetModel()->m_worldRow);
    Vector3 handPos = Vector3(offset.x, offset.y, offset.z);

    Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
    dir = Vector4::Transform(
        dir, GetModel()->m_worldRow);
    dir.Normalize();
    dir *= 1.5f / simToRenderScale;

    ProjectileManager::GetInstance().CreateProjectile(handPos,
        0.1 / simToRenderScale, Vector3(dir.x, dir.y, dir.z));
}
}