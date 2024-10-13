#pragma once
#include "GeometryGenerator.h"
#include "DModel.h"
#include "GameDef.h"
#include "Object.h"
#include <map>
namespace hlab {
	using std::function;
	class DModel;
	class ActorState;
	class DSkinnedMeshModel;
	//using std::map;
class Actor : public Object, public std::enable_shared_from_this<Actor> {
public:
	Actor();
	Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	virtual void Tick(float dt);
	
	// State
	void SetState(EActorStateType InType);
	EActorStateType GetPrevState() { return m_prevStateType; }
	// Camera
	void ActiveCaemera();
	void UpdateCemeraCorrection(Vector3 deltaPos);

	bool MsgProc(WPARAM wParam, bool bPress);
	
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);

	shared_ptr<ActorState> GetState() { return m_actorState; }

private:
	void UpdateState();
public:
	//ActorState GetActorState() { return m_actorState; }
protected:
	virtual void InitBoundingKey() {};
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
