#include "AttackState2.h"
#include "Actor2.h"

namespace dengine {

	AttackState::AttackState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = EActorStateType::Attack;
		m_loopState = false;
	}
	void AttackState::Initialize()
	{

	}
	void AttackState::Tick(float dt)
	{ 
		ActorState::Tick(dt);
	}
	void AttackState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		actorLock->RequestStateChange(EActorStateType::Idle);
	}
	// 인풋 받아 리천
	void AttackState::Transition()
	{
		Finish();
	}
}