#include "FlyAwayState2.h"
#include "Actor2.h"

namespace dengine {

	FlyAwayState::FlyAwayState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = EActorStateType::FlyAway;
		m_flyAwayState = EFlyAwayStateType::FlayAwayStateFlying;
		m_loopState = false;
	}
	void FlyAwayState::Initialize()
	{

	}
	void FlyAwayState::Tick(float dt)
	{
		ActorState::Tick(dt);
		UpdateVelocity(dt);
		PauseFrameIfFlyAway();
	}
	void FlyAwayState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		actorLock->SetState(EActorStateType::Idle);
		actorLock->ResetExternalForce();
	}
	// ��ǲ �޾� ��õ
	void FlyAwayState::Transition()
	{
		if (m_flyAwayState == EFlyAwayStateType::FlayAwayStateFlying)
		{
			m_flyAwayState = EFlyAwayStateType::FlayAwayStateLieDown;
			m_pauseFrame = false;
			std::shared_ptr<Actor> actorLock = m_actor.lock();
			if (actorLock.get() != nullptr)
			{
				actorLock->ResetExternalForce();
			}

		}
		else
		{
			Finish();
		}
	}

	void FlyAwayState::UpdateVelocity(float dt)
	{
		// ���� ���׿� ���� ���󰡴�
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		if (m_flyAwayState == EFlyAwayStateType::FlayAwayStateFlying)
		{
			if (m_elapsedTime > 1.0f)
			{
				Transition();
			}
		}
	}

	void FlyAwayState::PauseFrameIfFlyAway()
	{
		if (m_flyAwayState == EFlyAwayStateType::FlayAwayStateFlying && m_frame >= 18)
		{
			m_pauseFrame = true;
		}
	}
}