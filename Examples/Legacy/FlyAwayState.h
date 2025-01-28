#pragma once

#pragma once
#include "GameDef.h"
#include "ActorState.h"
namespace hlab {
	class Actor;
	class FlyAwayState : public ActorState
	{
	public:
		FlyAwayState() {}
		FlyAwayState(std::weak_ptr<Actor> inModel);

		virtual void Initialize();
		virtual void Tick(float dt);
		virtual void Finish();
		// ��ǲ �޾� ��õ
		virtual void Transition();

	private:
		void UpdateVelocity(float dt);
		void PauseFrameIfFlyAway();

	private:
		EFlyAwayStateType m_flyAwayState;
	};

}

