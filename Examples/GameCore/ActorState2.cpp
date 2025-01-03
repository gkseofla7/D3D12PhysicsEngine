#include "ActorState2.h"
#include "DModel2.h"
#include "AnimHelper2.h"
#include "DSkinnedMeshModel2.h"
#include "Actor2.h"
#include "magic_enum.hpp"

namespace dengine {

ActorState::ActorState(std::weak_ptr<Actor> InActor)
{	
	m_actor = InActor;
}
void ActorState::Tick(float dt)
{
	m_elapsedTime += dt;
	std::shared_ptr<Actor> actorLock = m_actor.lock();
	if (actorLock.get() == nullptr)
	{
		return;
	}
	std::shared_ptr<ActorState> myLock = actorLock->GetState();
	if (std::shared_ptr<DSkinnedMeshModel> derivedPtr = std::dynamic_pointer_cast<DSkinnedMeshModel>(actorLock->GetModel()))
	{
		if (m_pauseFrame == false)
		{
			UpdateAnimation();
			m_frame++;
			if (m_frame >= derivedPtr->GetMaxFrame())
			{
				m_frame = 0;
				if (false == m_loopState)
				{
					// 주의! 나의 shared_ptr을 참조하고 있는 유일한 곳을 끊어준다..
					Transition();
				}
			}
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
	std::shared_ptr<Actor> actorLock = m_actor.lock();
	if (actorLock.get() == nullptr)
	{
		return;
	}
	AnimHelper::GetInstance().UpdateAnimation(actorLock.get(), magic_enum::enum_name(m_state).data(), m_frame);
}

bool ActorState::ActionKeyIfBind(WPARAM InKey, bool InPressed)
{
	if (m_keyBindingPress.find(InKey) != m_keyBindingPress.end())
	{
		return m_keyBindingPress[InKey]();
	}
	return false;
}
}
