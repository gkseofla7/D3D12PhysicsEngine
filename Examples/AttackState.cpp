#include "AttackState.h"

namespace hlab {

	AttackState::AttackState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = ActorStateType::Attack;
		m_loopState = false;
		m_afterState = ActorStateType::Idle;
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

	}
	// 인풋 받아 리천
	ActorStateType AttackState::Transition()
	{
		return ActorStateType::Idle;
	}
}