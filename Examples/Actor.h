#pragma once
#include "Model.h"
namespace hlab {

	using std::function;
	using std::map;
class Actor {
public:
	void ActiveCaemera();
	bool MsgProc(WPARAM wParam);
	void UpdateWorldRow(const Matrix& worldRow);
	void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context);
	void UpdateCemeraCorrection(Vector3 deltaPos);
protected:

	shared_ptr<class Camera> m_camera;

	//Model Constant를 모두 actorConst에서 하도록
	ConstantBuffer<MeshConstants> m_actorConsts;
public:
	shared_ptr<Model> m_model;

	Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
	Matrix m_worldITRow = Matrix(); // InverseTranspose
	DirectX::BoundingSphere m_boundingSphere;
	bool m_isVisible = true;
	
	Matrix m_cameraCorrection;

	map< WPARAM, function<void()>> m_keyBinding;
};

} // namespace hlab
