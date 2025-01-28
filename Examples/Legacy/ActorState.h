#pragma once
#include "GameDef.h"
#include <memory>
#include <map>
#include <functional> 
#include <windows.h>
namespace hlab {

class Actor;
class ActorState
{

	//C++���� �߻� Ŭ������ �����ڿ��� �Լ��� ȣ���� ���� ������,
	//�����ؾ� �� �߿��� ������ �ֽ��ϴ�. �߻� Ŭ������ �ϳ� �̻���
	// ���� ���� �Լ�(pure virtual function)�� �����ϴ� Ŭ�����Դϴ�.
	// �߻� Ŭ������ �����ڿ��� ���� �Լ��� ȣ���ϴ� ���,
	// �ش� �Լ��� �Ļ� Ŭ�������� �����ǵ� �Լ��� �ִ���,
	//ȣ��Ǵ� �Լ��� �߻� Ŭ���� ��ü���� ���ǵ� �Լ��Դϴ�.
	//�̴� �����ڰ� ȣ��� �� �Ļ� Ŭ������ �����ڰ� ���� ȣ����� �ʾұ� �����Դϴ�.
public:
	ActorState(){}
	ActorState(std::weak_ptr<Actor> InActor);

	virtual void Initialize() = 0;
	virtual void Tick(float dt);
	virtual void Finish();
	virtual void Transition();
	virtual void UpdateAnimation();
	bool ActionKeyIfBind(WPARAM InKey, bool InPressed);

	EActorStateType GetStateType() { return m_state; }
	int GetFrame() { return m_frame; }
protected:
	EActorStateType m_state;
	bool m_loopState = false;
	int m_frame = 0;
	bool m_pauseFrame = false;

	std::weak_ptr<Actor> m_actor;

	float m_elapsedTime = 0.0f;
private:
	bool m_finished = false;

	std::map<WPARAM, std::function<bool()>> m_keyBindingPress;
	std::map<WPARAM, std::function<bool()>> m_keyBindingRelease;
	
};
}
