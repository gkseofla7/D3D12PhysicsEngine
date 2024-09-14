#pragma once
#include "GameDef.h"
#include "ActorState.h"
namespace hlab {
class MoveState : public ActorState
{
public:
	MoveState() {}
	MoveState(std::weak_ptr<Actor> InModel);

	virtual void Initialize();
	virtual void Tick();
	virtual void Finish();
	// 인풋 받아 리천
	virtual void Transition();

private:
	MoveStateType m_moveState;
};
}
