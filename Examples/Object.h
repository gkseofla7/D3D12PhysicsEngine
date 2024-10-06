#pragma once
#include "GameDef.h"
#include "D3D11Utils.h"
#include <memory>
#include <directxtk/SimpleMath.h>
class btRigidBody;
namespace hlab {
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;
class DModel;
class Model;
class DSkinnedMeshModel;
class BillboardModel;
class Object
{
public:
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	virtual void Tick(float dt);

	void SetPhysicsBody(btRigidBody* InPhysicsBody) { m_physicsBody = InPhysicsBody; }

	// 위치 이동 관련
	void UpdateWorldRow(const Matrix& worldRow);
	void UpdatePosition(const Vector3& InDelta);
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
	bool IsPickable() { return m_isPickable; }
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);

	shared_ptr<DModel> GetModel() { return m_model; }
	shared_ptr<DSkinnedMeshModel> GetSkinnedMeshModel();
	shared_ptr<BillboardModel> GetBillboardModel();
protected:
	std::shared_ptr<DModel> m_model;
	btRigidBody* m_physicsBody = nullptr;

	float m_velocity = 0.0f;
	bool m_isPickable = true; // 마우스로 선택/조작 가능 여부
};
}

