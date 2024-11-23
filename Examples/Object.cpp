#include "Object.h"
#include "DModel.h"
#include "BillboardModel.h"
#include "DSkinnedMeshModel.h"
#include "bullet/BulletDynamics/btBulletDynamicsCommon.h"
#include "BulletDynamics/btBulletDynamicsCommon.h"

namespace hlab {
    void Object::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> inModel)
    {
        m_objectId = ObjectNumberGenerator::GetInstance().GetNewObjectNumber();
        m_model = inModel;
    }
    void Object::Tick(float dt)
    {
        if (m_model)
        {
            m_model->Tick(dt);
        }
        if (IsUsePhsycsSimulation() == false)
        {
            if (m_physicsBody)
            {
                Vector3 worldPos = GetWorldPosition();
                static const float simToRenderScale = 0.01f;
                btTransform t = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(worldPos.x, worldPos.y, worldPos.z) /
                    simToRenderScale);
                m_physicsBody->setWorldTransform(t);
            }
        }
    }

    void Object::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        m_model->Render(context);
    }

    // 위치 이동 관련
    void Object::UpdateWorldRow(const Matrix& worldRow)
    {
        m_model->UpdateWorldRow(worldRow);
    }
    void Object::UpdatePosition(const Vector3& inDelta)
    {
        m_model->UpdatePosition(inDelta);
    }

    void Object::UpdateRotationY(float inDelta)
    {
        m_model->UpdateRotation(Matrix::CreateRotationY(inDelta));
    }
    void Object::UpdateRoationWithDirection(const Vector3& inLookingDir)
    {
        m_model->SetDirection(inLookingDir);
    }
    Vector3 Object::GetWorldPosition()
    {
        return m_model->GetWorldPosition();
    }
    shared_ptr<DSkinnedMeshModel> Object::GetSkinnedMeshModel()
    { 
        return std::dynamic_pointer_cast<DSkinnedMeshModel>(m_model);
    }

    shared_ptr<BillboardModel> Object::GetBillboardModel()
    {
        return std::dynamic_pointer_cast<BillboardModel>(m_model);
    }
    
    void Object::SubtractExternalForce(float inForceSub)
    {
        if (m_externalForce > 0.0f)
        {
            m_externalForce -= inForceSub;
        }
        if (m_externalForce < 0.0f)
        {
            m_externalForce = 0.0f;
        }
    }
    void Object::AddMomentum(const float inMomentum, Vector3 inDir)
    { 
        if (m_physicsBody == nullptr)
        {
            return; 
        }
        inDir.Normalize();
        // 3을 곱한 이유는 과한 효과를 위해
        m_externalForce = 3.0f*sqrt(inMomentum / m_physicsBody->getMass());
        UpdateRoationWithDirection(-inDir);
    }
}