#pragma once

#include "GeometryGenerator.h"
#include "Model.h"

namespace hlab {

using std::make_shared;

class SkinnedMeshModel : public Model {
  public:
    SkinnedMeshModel(ComPtr<ID3D11Device> &device,
                     ComPtr<ID3D11DeviceContext> &context,
                     const vector<MeshData> &meshes,
                     const AnimationData &aniData) {
        Initialize(device, context, meshes, aniData);
    }

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes,
                    const AnimationData &aniData) {
        InitAnimationData(device, aniData);
        Model::Initialize(device, context, meshes);
    }

    GraphicsPSO &GetPSO(const bool wired) override {
        //defaultSolidPSO
        //skinnedSolidPSO
        return wired ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
    }

    GraphicsPSO &GetReflectPSO(const bool wired) override {
        return wired ? Graphics::reflectSkinnedWirePSO
                     : Graphics::reflectSkinnedSolidPSO;
    }

    GraphicsPSO &GetDepthOnlyPSO() override {
        return Graphics::depthOnlySkinnedPSO;
    }

    void InitMeshBuffers(ComPtr<ID3D11Device> &device, const MeshData &meshData,
                         shared_ptr<Mesh> &newMesh) override {
        D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                                       newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
        newMesh->stride = UINT(sizeof(SkinnedVertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      newMesh->indexBuffer);
    }

    void InitAnimationData(ComPtr<ID3D11Device> &device,
                           const AnimationData &aniData) {
        if (!aniData.clips.empty()) {
            m_aniData = aniData;

            // 여기서는 AnimationClip이 SkinnedMesh라고 가정하겠습니다.
            // 일반적으로는 모든 Animation이 SkinnedMesh Animation은 아닙니다.
            m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

            // 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
            for (int i = 0; i < aniData.clips.front().keys.size(); i++)
                m_boneTransforms.m_cpu[i] = Matrix();
            m_boneTransforms.Initialize(device);
        }
    }

     
    void Render(ComPtr<ID3D11DeviceContext> &context) override {

        // ConstBuffer 대신 StructuredBuffer 사용
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());
        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의

        // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
        // Model::Render(.)를 같이 사용 가능

        Model::Render(context);
    };

    // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);
    void UpdateVelocity(float dt) { 
        Vector3 prevPos = m_prevRootTransform.Translation();
        Vector3 curPos = m_aniData.accumulatedRootTransform.Translation();

        m_velocity = (curPos - prevPos).Length()/dt;
        m_prevRootTransform = m_aniData.accumulatedRootTransform;
    }
  public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;

    AnimationData m_aniData;
    float m_velocity = 0.0f;
    Matrix m_prevRootTransform;
};

} // namespace hlab