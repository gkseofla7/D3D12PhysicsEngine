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
        // ConstBuffer ��� StructuredBuffer ���
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        //context->VSSetShaderResources(
        //    9, 1, m_boneTransforms->GetAddressOfSRV()); // �׻� slot index ����

        // Skinned VS/PS�� GetPSO()�� ���ؼ� �����Ǳ� ������
        // Model::Render(.)�� ���� ��� ����
        Actor::Render(context);
    };
}