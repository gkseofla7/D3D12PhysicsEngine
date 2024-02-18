#pragma once

#include "GeometryGenerator.h"
#include "DModel.h"

namespace hlab {

    using std::make_shared;
    // DModel에서는 Mesh Loading
    // DSkinnedMeshModel에서는 Animation Loading
    class DSkinnedMeshModel : public DModel {
    public:
        DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename);
        DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename,
            const string& animPath,
            const string& animFilename);
        virtual void Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath, const string& filename)override;

        GraphicsPSO& GetPSO(const bool wired) override {
            //defaultSolidPSO
            //skinnedSolidPSO
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
            shared_ptr<Mesh>& newMesh) override;

        virtual void UpdateAnimation(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            int clipId, int frame, int type = 0) override;

        virtual void Render(ComPtr<ID3D11DeviceContext>& context) override;

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
        AnimationData* m_aniData = nullptr;
        float m_velocity = 0.0f;
        Matrix m_prevRootTransform;
        Matrix m_accumulatedRootTransform;

    };

} // namespace hlab