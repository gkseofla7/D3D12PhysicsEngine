#pragma once
#include "Legacy/GameDef.h"
#include "ActorState.h"
namespace hlab {
	class Actor;
	class JumpState : public ActorState
	{
	public:
		JumpState() {}
		JumpState(std::weak_ptr<Actor> inModel);

		virtual void Initialize();
		virtual void Tick(float dt);
		virtual void Finish();
		virtual void Transition();
		virtual void UpdateAnimation();
	private:
		EJumpStateType m_jumpState;
	};

}