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
            string clipId, int frame, int type = 0) override;

        virtual void Render(ComPtr<ID3D11DeviceContext>& context) override;

        // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
        // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
        // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
        // &context);
//        virtual void UpdatePosition(const Vector3& inDelta)
//        {
///*            m_accumulatedRootTransform =
//                Matrix::CreateTranslation(inDelta) *
//                m_accumulatedRootTransform;    */       
//            m_accumulatedRootTransform =
//                Matrix::CreateTranslation(inDelta) *
//                m_accumulatedRootTransform;
//        }
        Matrix& GetAccumulatedRootTransform() { return m_accumulatedRootTransform; }
        Matrix& GetAccumulatedRootTransformToLocal() { return m_accumulatedRootTransformToLocal; }

        void SetAccumulatedRootTransform(const Matrix& InAccumulatedRootTransform) { m_accumulatedRootTransform = InAccumulatedRootTransform; }
        void SetAccumulatedRootTransformToLocal(const Matrix& InAccumulatedRootTransformToLocal) { m_accumulatedRootTransformToLocal = InAccumulatedRootTransformToLocal; }

        void IntegrateRootTransformToWorldTransform(ComPtr<ID3D11DeviceContext>& context);
    public:
        // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
        shared_ptr<StructuredBuffer<Matrix>> m_boneTransforms;
    private:
        Matrix m_accumulatedRootTransform;
        Matrix m_accumulatedRootTransformToLocal;

    };

} // namespace hlab