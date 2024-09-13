#pragma once
#include "GameDef.h"
#include "ActorState.h"
namespace hlab {
class Actor;
class AttackState : public ActorState
{
public:
	AttackState(){}
	AttackState(std::weak_ptr<Actor> InModel);

	virtual void Initialize();
	virtual void Tick();
	virtual void Finish();
	// 인풋 받아 리천
	virtual ActorStateType Transition();
};

}