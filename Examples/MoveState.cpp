#include "MoveState.h"
#include "Actor.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
#include "magic_enum.hpp"
namespace hlab {

	MoveState::MoveState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = EActorStateType::Move;
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		if (actorLock->GetVelocity() != 0.0f)
		{
			m_moveState = EMoveStateType::MoveStateWalk;
			m_loopState = true;
		}
		else
		{
			
			m_moveState = EMoveStateType::MoveStateIdleToWalk;
			m_loopState = false;
		}
	}
	void MoveState::Initialize()
	{
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		std::shared_ptr<ActorState> myLock = actorLock->GetState();
		if (std::shared_ptr<DSkinnedMeshModel> derivedPtr = std::dynamic_pointer_cast<DSkinnedMeshModel>(actorLock->GetModel()))
		{
			AnimHelper::GetInstance().LoadAnimation(derivedPtr.get(), magic_enum::enum_name(EMoveStateType::MoveStateIdleToWalk).data());
			AnimHelper::GetInstance().LoadAnimation(derivedPtr.get(), magic_enum::enum_name(EMoveStateType::MoveStateWalk).data());
			AnimHelper::GetInstance().LoadAnimation(derivedPtr.get(), magic_enum::enum_name(EMoveStateType::MoveStateWalkToIdle).data());
		}
	}
	void MoveState::Tick(float dt)
	{
		ActorState::Tick(dt);
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		actorLock->SetVelocity(0.0f);
		if (m_moveState == EMoveStateType::MoveStateWalk)
		{
			//actorLock.get()->UpdatePosition(Vector3(0., 0., -dt * 3.0f));
			actorLock->SetVelocity(3.0f);

			if ((bRoateLeft || bRotateRight) && bRoateLeft != bRotateRight) {
				float sign = bRotateRight == true ? 1.0f : -1.0f;
				actorLock.get()->UpdateRotationY(sign *
					(3.141592f * 60.0f / 180.0f * dt));

			}
		}
	}
	void MoveState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		actorLock->SetState(EActorStateType::Idle);
		// 이후 난 죽음..ㅋㅋ 액터 틱에서 펜딩 걸어주는게 좋을듯 보인다
	}
	// 인풋 받아 리천
	void MoveState::Transition()
	{
		if (m_moveState != EMoveStateType::EndOfEnum)
		{
			m_moveState = EMoveStateType((int)m_moveState + 1);
		}

		if (m_moveState == EMoveStateType::MoveStateWalk)
		{
			m_loopState = true;
		}
		else
		{
			m_loopState = false;
		}

		if (m_moveState == EMoveStateType::EndOfEnum)
		{
			Finish();
		}
	}
	void MoveState::UpdateAnimation()
	{
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		std::shared_ptr<ActorState> myLock = actorLock->GetState();
		if (std::shared_ptr<DSkinnedMeshModel> derivedPtr = std::dynamic_pointer_cast<DSkinnedMeshModel>(actorLock->GetModel()))
		{
			AnimHelper::GetInstance().UpdateAnimation(derivedPtr.get(), magic_enum::enum_name(m_moveState).data(), m_frame);
		}
	}
}