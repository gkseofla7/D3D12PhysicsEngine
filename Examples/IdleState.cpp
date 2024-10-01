#include "IdleState.h"
#include "Actor.h"
namespace hlab {

	IdleState::IdleState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = ActorStateType::Idle;
		m_loopState = true;
	}
	void IdleState::Initialize()
	{
		m_loopState = true;
	}
	void IdleState::Tick(float dt)
	{
		ActorState::Tick(dt);
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		actorLock->SetVelocity(0.0f);
	}
	void IdleState::Finish()
	{
		ActorState::Finish();
	}
}