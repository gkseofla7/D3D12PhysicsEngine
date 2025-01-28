#pragma once

#pragma once
#include "GameDef2.h"
#include "ActorState2.h"
namespace dengine {
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

