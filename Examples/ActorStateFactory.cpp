#include "ActorStateFactory.h"
#include "ActorState.h"
#include "IdleState.h"
#include "AttackState.h"
#include "Actor.h"
#include "MoveState.h"
#include "JumpState.h"
namespace hlab {
std::shared_ptr<ActorState> ActorStateFactory::CreateActorState(ActorStateType InType, std::shared_ptr<Actor> InActor)
{
	if (InType == ActorStateType::Idle)
	{
		return std::make_shared<IdleState>(InActor);
	}
	else if (InType == ActorStateType::Attack)
	{
		return std::make_shared<AttackState>(InActor);
	}
	else if (InType == ActorStateType::Move)
	{
		return std::make_shared<MoveState>(InActor);
	}
	else if (InType == ActorStateType::Jump)
	{
		return std::make_shared<JumpState>(InActor);
	}
	return std::make_shared<IdleState>(InActor);
}
}