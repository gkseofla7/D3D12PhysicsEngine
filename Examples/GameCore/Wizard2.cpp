#include "Wizard2.h"
#include "AnimHelper2.h"
#include "ActorStateFactory2.h"
#include "ActorState2.h"
//#include "ProjectileManager.h"
#include "DSkinnedMeshModel2.h"
#include "MoveState2.h"
#include "magic_enum.hpp"

namespace dengine {

Wizard::Wizard(shared_ptr<DModel> inModel)
    :Actor(inModel)
{
	//Initialize(device, context, inModel);
}
void Wizard::Initialize(shared_ptr<DModel> inModel)
{
    Actor::Initialize(inModel);
    m_model->SetModelId(1);

	InitBoundingKey();
    InitAnimPath();
}
 
void Wizard::Tick(float dt)
{
    Actor::Tick( dt);

    m_actorState->Tick(dt);

    if (m_actorState->GetStateType() == EActorStateType::Attack)
    {
        static const int FIreBallStartFrame = 115;
        if (m_actorState->GetFrame() == FIreBallStartFrame) {
            ShotFireBall();
        }
    }
    if (m_actorState->GetStateType() != EActorStateType::Move)
    {
        if (bLeft)
        {
            UpdateRotationY((3.141592f * 60.0f / 180.0f * dt));
        }
        if (bRight)
        {
            UpdateRotationY(-(3.141592f * 60.0f / 180.0f * dt));
        }
    }
}

void Wizard::ReactProjectileHitted()
{
    SetState(EActorStateType::FlyAway);
}

void Wizard::InitAnimPath()
{
    // 애니메이션 관련, 따로 테이블을 만들어서 관리하는게..
    string path = "../Assets/Characters/Mixamo/";
    AnimHelper::GetInstance().AddAnimPath(m_model->GetModelId(), path);
    // Idle
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EActorStateType::Idle).data(), "FightingIdleOnMichelle2.fbx");
    LoadAnimAsync(magic_enum::enum_name(EActorStateType::Idle).data());
    // Attack
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EActorStateType::Attack).data(), "Fireball.fbx");
    LoadAnimAsync(magic_enum::enum_name(EActorStateType::Attack).data());
    // Move
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EMoveStateType::MoveStateIdleToWalk).data(), "Female Start Walking.fbx");
    LoadAnimAsync(magic_enum::enum_name(EMoveStateType::MoveStateIdleToWalk).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EMoveStateType::MoveStateWalk).data(), "Walking.fbx");
    LoadAnimAsync(magic_enum::enum_name(EMoveStateType::MoveStateWalk).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EMoveStateType::MoveStateWalkToIdle).data(), "Female Stop Walking.fbx");
    LoadAnimAsync(magic_enum::enum_name(EMoveStateType::MoveStateWalkToIdle).data());
    // Jumping
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EJumpStateType::JumpStateInPlace).data(), "Jumping.fbx");
    LoadAnimAsync(magic_enum::enum_name(EJumpStateType::JumpStateInPlace).data());
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EJumpStateType::JumpStateRunning).data(), "Jump.fbx");
    LoadAnimAsync(magic_enum::enum_name(EJumpStateType::JumpStateRunning).data());
    
    // 쓰러짐
    AnimHelper::GetInstance().AddAnimStateToAnim(m_model->GetModelId(), magic_enum::enum_name(EActorStateType::FlyAway).data(), "Stunned.fbx");
    LoadAnimAsync(magic_enum::enum_name(EActorStateType::FlyAway).data());
}

void Wizard::LoadAnimAsync(string inState)
{
    AnimHelper::GetInstance().LoadAnimation(GetSkinnedMeshModel().get(), inState);
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
    if (m_actorState->GetStateType() == EActorStateType::Idle)
    {
        SetState(EActorStateType::Attack);
    }	
}
void Wizard::WalkStart()
{
    if (m_actorState->GetStateType() == EActorStateType::Idle)
    {
        SetState(EActorStateType::Move);
    }
}
void Wizard::WalkEnd()
{
    if (m_actorState->GetStateType() == EActorStateType::Move)
    {
        m_actorState->Finish();
    }
    else if (m_prevStateType == EActorStateType::Move)
    {
        m_prevStateType = EActorStateType::Idle;
    }
}

void Wizard::RotateLeft(bool InOn)
{
    bLeft = InOn;
    if (m_actorState->GetStateType() == EActorStateType::Move)
    {
        std::shared_ptr<MoveState> derivedPtr = std::dynamic_pointer_cast<MoveState>(m_actorState);
        derivedPtr->RotateLeft(InOn);
    }
}
void Wizard::RotateRight(bool InOn)
{
    bRight = InOn;
    if (m_actorState->GetStateType() == EActorStateType::Move)
    {
        std::shared_ptr<MoveState> derivedPtr = std::dynamic_pointer_cast<MoveState>(m_actorState);
        derivedPtr->RotateRight(InOn);
    }
}

void Wizard::Jump()
{
    SetState(EActorStateType::Jump);
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

    //ProjectileManager::GetInstance().CreateProjectile(handPos,
    //    0.1 / simToRenderScale, Vector3(dir.x, dir.y, dir.z));
}
}