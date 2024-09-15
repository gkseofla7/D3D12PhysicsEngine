#pragma once
#include "GeometryGenerator.h"
#include "DModel.h"
#include "GameDef.h"
#include <map>
namespace hlab {
	using std::function;
	class DModel;
	class ActorState;
	//using std::map;
class Actor : public std::enable_shared_from_this<Actor> {
public:
	Actor();
	Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	//TODO. device랑 context를 안건네줄 방법을 찾아보자..
	virtual void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt);
	void UpdatePosition(const Vector3& InDelta);
	void UpdateVelocity(float dt);
	void SetState(ActorStateType InType);
	void ActiveCaemera();

	bool MsgProc(WPARAM wParam, bool bPress);
	void UpdateCemeraCorrection(Vector3 deltaPos);
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);

	shared_ptr<DModel> GetModel() { return m_model; }
	shared_ptr<ActorState> GetState() { return m_actorState; }
private:
	void UpdateState();

public:
	//ActorState GetActorState() { return m_actorState; }
protected:
	virtual void InitBoundingKey() {};
protected:
	shared_ptr<class Camera> m_camera;

protected:

	
	Matrix m_cameraCorrection;

	std::map<WPARAM, function<void()>> m_keyBindingPress;
	std::map<WPARAM, function<void()>> m_keyBindingRelease;
	//ActorState m_actorState;
protected:
	// ConstantBuffer<SkinnedConsts> m_skinnedConsts;
	shared_ptr<ActorState> m_actorState;
	ActorStateType m_actorStateType;
	shared_ptr<DModel> m_model;
	
};

} // namespace hlab
