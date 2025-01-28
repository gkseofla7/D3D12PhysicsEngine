#pragma once
#include "GameDef2.h"
#include "ActorState2.h"
namespace dengine {
class Actor;
class AttackState : public ActorState
{
public:
	AttackState(){}
	AttackState(std::weak_ptr<Actor> inModel);

	virtual void Initialize();
	virtual void Tick(float dt);
	virtual void Finish();
	// ��ǲ �޾� ��õ
	virtual void Transition();
};

}