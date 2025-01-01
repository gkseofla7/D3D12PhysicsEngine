#pragma once
#include "GameDef2.h"
#include "ActorState2.h"
namespace dengine {

class Actor;
class IdleState : public ActorState
{
public:
	IdleState() {}
	IdleState(std::weak_ptr<Actor> InActor);

	virtual void Initialize();
	virtual void Tick(float dt);
	virtual void Finish();
};

}