#pragma once
#include <memory>
#include "Legacy/GameDef.h"
namespace hlab {
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