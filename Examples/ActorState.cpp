#include "ActorState.h"
#include "DModel.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
#include "Actor.h"
#include "magic_enum.hpp"

namespace hlab {

ActorState::ActorState(std::weak_ptr<Actor> InActor)
{	m_actor = InActor;
}
void ActorState::Tick()
{
	std::shared_ptr<Actor> actorLock = m_actor.lock();
	if (actorLock.get() == nullptr)
	{
		return;
	}
	std::shared_ptr<ActorState> myLock = actorLock->GetState();
	if (std::shared_ptr<DSkinnedMeshModel> derivedPtr = std::dynamic_pointer_cast<DSkinnedMeshModel>(actorLock->GetModel()))
	{
		
		AnimHelper::GetInstance().UpdateAnimation(derivedPtr.get(), magic_enum::enum_name(m_state).data(), m_frame);

		//TODO. Loop 시스템인지 확인. 아니라면 Transition 필요
		if (m_frame == derivedPtr->m_maxFrame)
		{
			m_frame = 0;
			if (false == m_loopState)
			{
				// 주의! 나의 shared_ptr을 참조하고 있는 유일한 곳을 끊어준다..
				Transition();
			}
		}
		else
		{
			m_frame++;
		}
	}
}
void ActorState::Finish()
{
	
}

void ActorState::Transition()
{
	Finish();
}

void ActorState::UpdateAnimation()
{

}
}
