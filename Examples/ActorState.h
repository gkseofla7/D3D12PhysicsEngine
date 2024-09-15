#pragma once
#include "GameDef.h"
#include <memory>
namespace hlab {

class Actor;
class ActorState
{

	//C++에서 추상 클래스의 생성자에서 함수를 호출할 수는 있지만,
	//주의해야 할 중요한 사항이 있습니다. 추상 클래스는 하나 이상의
	// 순수 가상 함수(pure virtual function)를 포함하는 클래스입니다.
	// 추상 클래스의 생성자에서 가상 함수를 호출하는 경우,
	// 해당 함수가 파생 클래스에서 재정의된 함수가 있더라도,
	//호출되는 함수는 추상 클래스 자체에서 정의된 함수입니다.
	//이는 생성자가 호출될 때 파생 클래스의 생성자가 아직 호출되지 않았기 때문입니다.
public:
	ActorState(){}
	ActorState(std::weak_ptr<Actor> InActor);

	virtual void Initialize() = 0;
	virtual void Tick(float dt);
	virtual void Finish();
	// 인풋 받아 리천
	virtual void Transition();
	virtual void UpdateAnimation();

	ActorStateType GetStateType() { return m_state; }
	int GetFrame() { return m_frame; }
protected:
	ActorStateType m_state;
	bool m_loopState = false;
	int m_frame = 0;

	std::weak_ptr<Actor> m_actor;
private:
	bool m_finished = false;
	
};
}
