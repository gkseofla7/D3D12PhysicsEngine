#include "IdleState.h"

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
	}
	void IdleState::Finish()
	{
		ActorState::Finish();
	}
}