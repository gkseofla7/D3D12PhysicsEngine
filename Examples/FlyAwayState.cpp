#include "FlyAwayState.h"
#include "Actor.h"

namespace hlab {

	FlyAwayState::FlyAwayState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = ActorStateType::FlyAway;
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
		actorLock->SetState(ActorStateType::Idle);
	}
	// 인풋 받아 리천
	void FlyAwayState::Transition()
	{
		if (m_flyAwayState == FlyAwayStateType::FlayAwayStateFlying)
		{
			m_flyAwayState = FlyAwayStateType::FlayAwayStateLieDown;
			m_pauseFrame = false;
		}
		else
		{
			Finish();
		}
	}

	void FlyAwayState::UpdateVelocity(float dt)
	{
		// 공기 저항에 의해 날라가는
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		if (m_flyAwayState == FlyAwayStateType::FlayAwayStateFlying)
		{
			actorLock.get()->UpdateVelocity(-dt*0.1f);
			if (actorLock.get()->GetVelocity() == 0.0f)
			{
				Transition();
			}
		}
	}

	void FlyAwayState::PauseFrameIfFlyAway()
	{
		if (m_flyAwayState == FlyAwayStateType::FlayAwayStateFlying && m_frame >= 8)
		{
			m_pauseFrame = true;
		}
	}
}