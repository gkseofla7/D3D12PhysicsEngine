#include "InputController.h"
#include "Actor2.h"
namespace dengine {
void InputController::Possess(shared_ptr<Actor> inActor)
{
	m_ownedActor = inActor;
}

LRESULT InputController::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (m_ownedActor != nullptr)
		{
			if (m_ownedActor->MsgProc(wParam, true))
			{
				return true;
			}
		}
		m_keyPressed[wParam] = true;
		break;
	case WM_KEYUP:
		if (m_ownedActor != nullptr)
		{
			if (m_ownedActor->MsgProc(wParam, false))
			{
				return true;
			}
		}
		m_keyPressed[wParam] = false;
		break;
	}

	return false;
}
}