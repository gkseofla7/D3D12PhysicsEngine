#include "Object.h"
#include "DModel.h"
#include "BillboardModel.h"
#include "DSkinnedMeshModel.h"

#include "DaerimsEngineBase.h"
namespace hlab {

    void Object::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
    {
        m_model = InModel;
    }
    void Object::Tick(float dt)
    {
        if (m_physicsBody != nullptr)
        {
            if (m_physicsBody->hasContactResponse())
            {
                int a = 1;
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

    shared_ptr<DSkinnedMeshModel> Object::GetSkinnedMeshModel()
    { 
        return std::dynamic_pointer_cast<DSkinnedMeshModel>(m_model);
    }

    shared_ptr<BillboardModel> Object::GetBillboardModel()
    {
        return std::dynamic_pointer_cast<BillboardModel>(m_model);
    }
}