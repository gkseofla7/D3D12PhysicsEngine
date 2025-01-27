#pragma once
#include "Legacy/GameDef.h"
#include "ActorState.h"
namespace hlab {
class MoveState : public ActorState
{
public:
	MoveState() {}
	MoveState(std::weak_ptr<Actor> inModel);

	virtual void Initialize();
	virtual void Tick(float dt);
	virtual void Finish();
	// 인풋 받아 리천
	virtual void Transition();
	virtual void UpdateAnimation();

	void RotateLeft(bool InOn) { bRoateLeft = InOn; }
	void RotateRight(bool InOn) { bRotateRight = InOn; }
private:
	EMoveStateType m_moveState;
	bool bRoateLeft = false;
	bool bRotateRight = false;
};
}
