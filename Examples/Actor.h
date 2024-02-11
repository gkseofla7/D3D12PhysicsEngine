#pragma once
#include "GeometryGenerator.h"
#include "DModel.h"
#include <map>
namespace hlab {
	enum ActorState {
		NormalState = 1,
		SpecialState = 2,
		AnimNum,
	};
	using std::function;
	class DModel;
	//using std::map;
class Actor {
public:
	Actor();
	Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	//TODO. device랑 context를 안건네줄 방법을 찾아보자..
	virtual void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt) {}
	void ActiveCaemera();
	bool MsgProc(WPARAM wParam);
	void UpdateCemeraCorrection(Vector3 deltaPos);
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);
public:
	ActorState GetActorState() { return m_actorState; }
protected:
	virtual void InitBoundingKey() {};
protected:
	shared_ptr<class Camera> m_camera;

protected:
	shared_ptr<DModel> m_model;
	
	Matrix m_cameraCorrection;

	std::map<WPARAM, function<void()>> m_keyBinding;
	ActorState m_actorState;
public:
	// ConstantBuffer<SkinnedConsts> m_skinnedConsts;

};

} // namespace hlab
