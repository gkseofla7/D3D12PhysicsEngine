#pragma once
#include "GeometryGenerator2.h"
#include "DModel2.h"
#include "Object2.h"
#include "GameDef2.h"
#include <map>
namespace dengine {
	using std::function;
	class DModel;
	class ActorState;
	class DSkinnedMeshModel;
	//using std::map;
class Actor : public Object, public std::enable_shared_from_this<Actor> {
public:
	
	Actor();
	Actor(shared_ptr<DModel> inModel);
	virtual void Initialize(shared_ptr<DModel> inModel);
	virtual void Tick(float dt);
	void RequestStateChange(EActorStateType InType);
	EActorStateType GetPrevState() { return m_prevStateType; }
	// Camera
	void ActiveCamera();
	void UpdateCameraCorrection(Vector3 deltaPos);

	bool MsgProc(WPARAM wParam, bool bPress);
	
	virtual void Render();

	shared_ptr<ActorState> GetState() { return m_actorState; }

private:
	void UpdateState();
public:
protected:
	virtual void InitBoundingKey() {};
	void SetState(EActorStateType InType);
protected:
	shared_ptr<class Camera> m_camera;
	Matrix m_cameraCorrection;

	std::map<WPARAM, function<void()>> m_keyBindingPress;
	std::map<WPARAM, function<void()>> m_keyBindingRelease;
	// Actor에서 State가 어떤 동작을 하는지
	// 어떤 상태인지 모르고 돌아가는게 베스트 아닌가싶다.
	shared_ptr<ActorState> m_actorState;
	EActorStateType m_actorStateType;
	EActorStateType m_prevStateType;
};

} // namespace hlab
