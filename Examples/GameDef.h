#pragma once
namespace hlab {
// Main Type
enum class ActorStateType
{
	Idle,
	Attack,
	Move,
	Jump,
	FlyAway,
};

// Sub Type
enum class MoveStateType
{
	MoveStateIdleToWalk,
	MoveStateWalk,
	MoveStateWalkToIdle,

	EndOfEnum
};

enum class JumpStateType
{
	JumpStateInPlace,
	JumpStateRunning,
};

enum class FlyAwayStateType
{
	FlayAwayStateFlying,
	FlayAwayStateLieDown,
};

}