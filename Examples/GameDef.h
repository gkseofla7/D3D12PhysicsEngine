#pragma once
namespace hlab {
// Main Type
enum class EActorStateType
{
	Idle,
	Attack,
	Move,
	Jump,
	FlyAway,
};

// Sub Type
enum class EMoveStateType
{
	MoveStateIdleToWalk,
	MoveStateWalk,
	MoveStateWalkToIdle,

	EndOfEnum
};

enum class EJumpStateType
{
	JumpStateInPlace,
	JumpStateRunning,
};

enum class EFlyAwayStateType
{
	FlayAwayStateFlying,
	FlayAwayStateLieDown,
};


enum class ELoadType
{
	NotLoaded,
	Loading,
	Loaded,
};
}