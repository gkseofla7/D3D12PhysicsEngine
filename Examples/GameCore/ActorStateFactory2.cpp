#include "ActorStateFactory2.h"
#include "ActorState2.h"
#include "IdleState2.h"
//#include "AttackState.h"
//#include "Actor.h"
//#include "MoveState.h"
//#include "JumpState.h"
//#include "FlyAwayState.h"
namespace dengine {
std::shared_ptr<ActorState> ActorStateFactory::CreateActorState(EActorStateType InType, std::shared_ptr<Actor> InActor)
{
	switch (InType)
	{
	case  EActorStateType::Idle:
	{
		return std::make_shared<IdleState>(InActor);
	}
	//case  EActorStateType::Attack:
	//{
	//	return std::make_shared<AttackState>(InActor);
	//}
	//case  EActorStateType::Move:
	//{
	//	return std::make_shared<MoveState>(InActor);
	//}
	//case  EActorStateType::Jump:
	//{
	//	return std::make_shared<JumpState>(InActor);
	//}
	//case  EActorStateType::FlyAway:
	//{
	//	return std::make_shared<FlyAwayState>(InActor);
	//}
	}
	return std::make_shared<IdleState>(InActor);
}
}