#pragma once
#include <memory>
#include <windows.h>

namespace dengine {
using namespace std;
class Actor;
class InputController
{
public:
	void Possess(shared_ptr<Actor> inActor);
	LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	shared_ptr<Actor> m_ownedActor;
	bool m_keyPressed[256] = { false, };
};
}
