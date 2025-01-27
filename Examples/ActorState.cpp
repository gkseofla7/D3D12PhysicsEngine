#include "ActorState.h"
#include "Legacy/DModel.h"
#include "Legacy/AnimHelper.h"
#include "Legacy/DSkinnedMeshModel.h"
#include "Legacy/Actor.h"
#include "magic_enum.hpp"

namespace hlab {

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
			if (m_frame >= derivedPtr->m_maxFrame)
			{
				m_frame = 0;
				if (false == m_loopState)
				{
					// ����! ���� shared_ptr�� �����ϰ� �ִ� ������ ���� �����ش�..
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
