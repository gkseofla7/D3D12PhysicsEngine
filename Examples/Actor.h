#pragma once
#include "GeometryGenerator.h"
#include "Model.h"
#include <map>
namespace hlab {
	enum ActorState {
		NormalState = 1,
		SpecialState = 2,
		AnimNum,
	};
	using std::function;
	//using std::map;
class Actor {
public:
	Actor();
	virtual void Initialize();
	void ActiveCaemera();
	bool MsgProc(WPARAM wParam, shared_ptr<Actor> InActivateActore);
	void UpdateWorldRow(const Matrix& worldRow);
	void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context);
	void UpdateCemeraCorrection(Vector3 deltaPos);
	const int getActorId() const { return m_actorId; }
public:
	ActorState GetActorState() { return m_actorState; }
protected:
	virtual void InitBoundingKey() {};
protected:
	shared_ptr<class Camera> m_camera;

	//Model Constant를 모두 actorConst에서 하도록
	ConstantBuffer<MeshConstants> m_actorConsts;
protected:
	shared_ptr<Model> m_model;

	Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
	Matrix m_worldITRow = Matrix(); // InverseTranspose
	DirectX::BoundingSphere m_boundingSphere;
	bool m_isVisible = true;
	
	Matrix m_cameraCorrection;

	std::map<WPARAM, function<void(shared_ptr<Actor>)>> m_keyBinding;
	ActorState m_actorState;
	int m_actorId = 0;

public:
	// ConstantBuffer<SkinnedConsts> m_skinnedConsts;

};

} // namespace hlab
