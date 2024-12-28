#pragma once
#include <memory>
#include "GameDef2.h"
namespace dengine {
	class ActorState;
	class Actor;
class ActorStateFactory
{
private:
	ActorStateFactory() {};
public:
	static ActorStateFactory& GetInstance()
	{
		static ActorStateFactory helper;
		return helper;
	}
	std::shared_ptr<ActorState> CreateActorState(EActorStateType InType, std::shared_ptr<Actor> InActor);
};

}