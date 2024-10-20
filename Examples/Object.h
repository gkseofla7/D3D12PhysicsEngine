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
class ObjectNumberGenerator
{
public:
	static ObjectNumberGenerator& GetInstance()
	{
		static ObjectNumberGenerator generator;
		return generator;
	}
	int GetNewObjectNumber()
	{
		m_lastObjectId++;
		return m_lastObjectId;
	}
private:
	int m_lastObjectId = 0;
};

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
	void SetPendingKill(bool InPendingKill) { m_pendingKill = InPendingKill; }
	Vector3 GetWorldPosition();
	float GetVelocity() { return m_velocity; }

	void AddEnergy(const float InEnergy, Vector3 InDir);

	bool IsPickable() { return m_isPickable; }
	virtual void Render(ComPtr<ID3D11DeviceContext>& context);

	virtual bool IsNeedRegisterPhysics() { return m_needRegisterPhysics; }
	void ResetNeedRegisterPhysics() { m_needRegisterPhysics = false; }

	shared_ptr<DModel> GetModel() { return m_model; }
	shared_ptr<DSkinnedMeshModel> GetSkinnedMeshModel();
	shared_ptr<BillboardModel> GetBillboardModel();
	btRigidBody* GetPhysicsBody() { return m_physicsBody; }

	int GetObjectId() { return m_objectId; }

	void SetUsePhsycisSimulation(bool InUse) { m_usePhysicsSimulation = InUse; }

	bool IsPendingKill() { return m_pendingKill; }
	bool IsUsePhsycsSimulation() { return m_usePhysicsSimulation; }
protected:
	std::shared_ptr<DModel> m_model;
	btRigidBody* m_physicsBody = nullptr;

	float m_velocity = 0.0f;
	bool m_isPickable = false; // 마우스로 선택/조작 가능 여부

	bool m_needRegisterPhysics = false;
	bool m_usePhysicsSimulation = false;
	int m_objectId = 0;
private:
	bool m_pendingKill = false;
};
}

