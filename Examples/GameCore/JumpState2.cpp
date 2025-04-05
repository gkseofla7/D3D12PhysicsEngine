#include "JumpState2.h"
#include "Actor2.h"
#include "AnimHelper2.h"
#include "magic_enum.hpp"
#include "DSkinnedMeshModel2.h"
namespace dengine {

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
			actorLock->RequestStateChange(EActorStateType::Idle);
		}
		else if (m_jumpState == EJumpStateType::JumpStateRunning)
		{
			actorLock->RequestStateChange(EActorStateType::Move);
		}
	}
	// 인풋 받아 리천
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