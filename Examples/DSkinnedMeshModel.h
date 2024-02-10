#pragma once

#include "GeometryGenerator.h"
#include "DModel.h"
#include "AnimHelper.h"

namespace hlab {

    using std::make_shared;
    // DModel에서는 Mesh Loading
    // DSkinnedMeshModel에서는 Animation Loading
    class DSkinnedMeshModel : public DModel {
    public:
        DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename)
        {
            Initialize(device, context, basePath, filename);
        }
        DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename,
            const string& animPath,
            const string& animFilename)
        {
            Initialize(device, context, basePath, filename);
        }
        virtual void Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath, const string& filename)override
        {
            DModel::Initialize(device, context, basePath, filename);
        }

        GraphicsPSO& GetPSO(const bool wired) override {
            return wired ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
        }

        GraphicsPSO& GetReflectPSO(const bool wired) override {
            return wired ? Graphics::reflectSkinnedWirePSO
                : Graphics::reflectSkinnedSolidPSO;
        }

        GraphicsPSO& GetDepthOnlyPSO() override {
            return Graphics::depthOnlySkinnedPSO;
        }

        void InitMeshBuffers(ComPtr<ID3D11Device>& device, const MeshData& meshData,
            shared_ptr<Mesh>& newMesh) override {
            D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                newMesh->vertexBuffer);
            newMesh->indexCount = UINT(meshData.indices.size());
            newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
            newMesh->stride = UINT(sizeof(SkinnedVertex));
            D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                newMesh->indexBuffer);
        }

        void UpdateAnimation(ComPtr<ID3D11Device>& device, 
            ComPtr<ID3D11DeviceContext>& context,
            int clipId, int frame, int type = 0){
            AnimHelper::UpdateAnimation(device, context, this, clipId, frame);
        }

        void Render(ComPtr<ID3D11DeviceContext>& context) override {

            // ConstBuffer 대신 StructuredBuffer 사용
            // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

            context->VSSetShaderResources(
                9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의

            // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
            // Model::Render(.)를 같이 사용 가능

            DModel::Render(context);
        };

        // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
        // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
        // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
        // &context);
        void UpdateVelocity(float dt) 
        {
            //Vector3 prevPos = m_prevRootTransform.Translation();
            //Vector3 curPos = m_aniData.accumulatedRootTransform.Translation();

            //m_velocity = (curPos - prevPos).Length() / dt;
            //m_prevRootTransform = m_aniData.accumulatedRootTransform;
        }
    public:
        // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
        StructuredBuffer<Matrix> m_boneTransforms;
        int m_modelId;
        AnimationData* m_aniData = nullptr;
        float m_velocity = 0.0f;
        Matrix m_prevRootTransform;
    };

} // namespace hlab