#include "JumpState.h"
#include "Legacy/Actor.h"
#include "Legacy/AnimHelper.h"
#include "magic_enum.hpp"
#include "Legacy/DSkinnedMeshModel.h"
namespace hlab {

	JumpState::JumpState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = EActorStateType::Jump;
		m_loopState = false;
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		const float MinVelocity = 0.2f;
		if (actorLock.get()->GetPrevState() == EActorStateType::Move && actorLock.get()->GetVelocity() >= MinVelocity)
		{
			m_jumpState = EJumpStateType::JumpStateRunning;
		}
		else
		{
			m_jumpState = EJumpStateType::JumpStateInPlace;
		}
	}
	void JumpState::Initialize()
	{

	}
	void JumpState::Tick(float dt)
	{
		ActorState::Tick(dt);
	}
	void JumpState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		if (m_jumpState == EJumpStateType::JumpStateRunning)
		{
			if (actorLock.get()->GetPrevState() != EActorStateType::Move)
			{
				m_jumpState = EJumpStateType::JumpStateInPlace;
			}
		}

		if (m_jumpState == EJumpStateType::JumpStateInPlace)
		{
			actorLock->SetState(EActorStateType::Idle);
		}
		else if (m_jumpState == EJumpStateType::JumpStateRunning)
		{
			actorLock->SetState(EActorStateType::Move);
		}
	}
	// ��ǲ �޾� ��õ
	void JumpState::Transition()
	{
		Finish();
	}

	void JumpState::UpdateAnimation()
	{
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		AnimHelper::GetInstance().UpdateAnimation(actorLock.get(), magic_enum::enum_name(m_jumpState).data(), m_frame);
	}
}