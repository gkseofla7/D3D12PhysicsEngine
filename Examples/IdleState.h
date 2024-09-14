#pragma once
#include "GameDef.h"
#include "ActorState.h"
namespace hlab {

class Actor;
class IdleState : public ActorState
{
public:
	IdleState() {}
	IdleState(std::weak_ptr<Actor> InActor);

	virtual void Initialize();
	virtual void Tick();
	virtual void Finish();
};

}