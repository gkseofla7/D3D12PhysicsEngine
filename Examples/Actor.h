#pragma once
#include "GeometryGenerator.h"
#include "DModel.h"
#include "GameDef.h"
#include <map>
namespace hlab {
	using std::function;
	class DModel;
	class ActorState;
	class DSkinnedMeshModel;
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
	
	// 위치 이동 관련
	void UpdatePosition(const Vector3& InDelta);
	void UpdateVelocity(float InDelta);
	void UpdateRotationY(float InDelta);
	void SetVelocity(float InVelocity) { 
		if (InVelocity < 0.0)
		{
			m_velocity = 0.0;
			return;
		}
		m_velocity = InVelocity; 
	}
	float GetVelocity() { return m_velocity; }
	void SetState(ActorStateType InType);
	ActorStateType GetPrevState() { return m_prevStateType; }
	// 카메라 관련
	void ActiveCaemera();
	void UpdateCemeraCorrection(Vector3 deltaPos);

	bool MsgProc(WPARAM wParam, bool bPress);
	
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);

	shared_ptr<DModel> GetModel() { return m_model; }
	virtual shared_ptr<DSkinnedMeshModel> GetSkinnedMeshModel() { return nullptr; };
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
	ActorStateType m_actorStateType;
	ActorStateType m_prevStateType;
	shared_ptr<DModel> m_model;

	float m_velocity = 0.0f;
};

} // namespace hlab
