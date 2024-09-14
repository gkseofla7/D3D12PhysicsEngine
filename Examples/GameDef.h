#pragma once
namespace hlab {
// Main Type
enum class ActorStateType
{
	Idle,
	Attack,
	Move,
};

// Sub Type
enum class MoveStateType
{
	MoveStateIdleToWalk,
	MoveStateWalk,
	MoveStateWalkToIdle,

	EndOfEnum
};
}