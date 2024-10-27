#include"SkeletalMeshActor.h"
#include "DSkinnedMeshModel.h"
#include "AnimHelper.h"

namespace hlab {
    SkeletalMeshActor::SkeletalMeshActor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> inModel)
        :Actor(device, context, inModel)
    {
    }
    void SkeletalMeshActor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  inModel)
    {
        Actor::Initialize(device, context, inModel);
    }
    void SkeletalMeshActor::Tick(float dt)
    {
        Actor::Tick( dt);
    }

    void SkeletalMeshActor::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        // ConstBuffer 대신 StructuredBuffer 사용
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        //context->VSSetShaderResources(
        //    9, 1, m_boneTransforms->GetAddressOfSRV()); // 항상 slot index 주의

        // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
        // Model::Render(.)를 같이 사용 가능
        Actor::Render(context);
    };
}