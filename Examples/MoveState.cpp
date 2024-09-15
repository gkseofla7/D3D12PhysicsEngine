#include "MoveState.h"
#include "Actor.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
#include "magic_enum.hpp"
namespace hlab {

	MoveState::MoveState(std::weak_ptr<Actor> InActor)
		:ActorState(InActor)
	{
		m_state = ActorStateType::Move;
		m_moveState = MoveStateType::MoveStateIdleToWalk;
		m_loopState = false;
	}
	void MoveState::Initialize()
	{

	}
	void MoveState::Tick(float dt)
	{
		ActorState::Tick(dt);
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		if (actorLock.get() == nullptr)
		{
			return;
		}
		//bool rightPressed = AppBase::m_keyPressed[VK_RIGHT];
		//bool leftPressed = AppBase::m_keyPressed[VK_LEFT];

		//if ((rightPressed || leftPressed) && rightPressed != leftPressed) {
		//	float sign = rightPressed == true ? 1.0f : -1.0f;
		//	m_character->m_aniData.accumulatedRootTransform =
		//		Matrix::CreateRotationY(sign *
		//			(3.141592f * 60.0f / 180.0f * dt)) *
		//		m_character->m_aniData.accumulatedRootTransform;
		//}
		if (m_moveState == MoveStateType::MoveStateWalk)
		{
			actorLock.get()->UpdatePosition(Vector3(0., 0., -dt * 300.0f));
			actorLock->UpdateVelocity(dt);
		}

	}
	void MoveState::Finish()
	{
		ActorState::Finish();
		std::shared_ptr<Actor> actorLock = m_actor.lock();
		actorLock->SetState(ActorStateType::Idle);
		// 이후 난 죽음..ㅋㅋ 액터 틱에서 펜딩 걸어주는게 좋을듯 보인다
	}
	// 인풋 받아 리천
	void MoveState::Transition()
	{
		if (m_moveState != MoveStateType::EndOfEnum)
		{
			m_moveState = MoveStateType((int)m_moveState + 1);
		}

		if (m_moveState == MoveStateType::MoveStateWalk)
		{
			m_loopState = true;
		}
		else
		{
			m_loopState = false;
		}

		if (m_moveState == MoveStateType::EndOfEnum)
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