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
		const string& basePath, const string& filename);
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const string& basePath, const string& filename);
	void ActiveCaemera();
	bool MsgProc(WPARAM wParam, shared_ptr<Actor> InActivateActore);
	void UpdateWorldRow(const Matrix& worldRow);
	void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context);
	void UpdateCemeraCorrection(Vector3 deltaPos);
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);
	const int getActorId() const { return m_actorId; }
public:
	ActorState GetActorState() { return m_actorState; }
protected:
	virtual void InitBoundingKey() {};
protected:
	shared_ptr<class Camera> m_camera;

protected:
	DModel m_model;
	
	Matrix m_cameraCorrection;

	std::map<WPARAM, function<void()>> m_keyBinding;
	ActorState m_actorState;
	int m_actorId = 0;
public:
	// ConstantBuffer<SkinnedConsts> m_skinnedConsts;

};

} // namespace hlab
