#include "Object.h"
#include "DModel.h"
#include "BillboardModel.h"
#include "DSkinnedMeshModel.h"
#include "BulletDynamics/btBulletDynamicsCommon.h"

namespace hlab {
    void Object::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
    {
        m_objectId = ObjectNumberGenerator::GetInstance().GetNewObjectNumber();
        m_model = InModel;
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
                Vector3 WorldPos = GetWorldPosition();
                static const float simToRenderScale = 0.01f;
                btTransform t = btTransform(btQuaternion(), btVector3(WorldPos.x, WorldPos.y, WorldPos.z) /
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
    void Object::UpdatePosition(const Vector3& InDelta)
    {
        m_model->UpdatePosition(InDelta);
    }

    void Object::UpdateRotationY(float InDelta)
    {
        m_model->UpdateRotation(Matrix::CreateRotationY(InDelta));
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

    void Object::AddEnergy(const float InEnergy, Vector3 InDir)
    {
        if (m_physicsBody == nullptr)
        {
            return;
        }
        InDir.Normalize();
        Vector3 Force = sqrt(InEnergy / m_physicsBody->getMass()) * InDir;
    }
}