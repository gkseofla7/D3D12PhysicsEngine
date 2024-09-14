#include "AttackState.h"
#include "Actor.h"

namespace hlab {

	AttackState::AttackState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = ActorStateType::Attack;
		m_loopState = false;
	}
	void AttackState::Initialize()
	{

	}
	void AttackState::Tick()
	{ 
		ActorState::Tick();
	}
	void AttackState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		actorLock->SetState(ActorStateType::Idle);
	}
	// 인풋 받아 리천
	void AttackState::Transition()
	{
		Finish();
	}
}