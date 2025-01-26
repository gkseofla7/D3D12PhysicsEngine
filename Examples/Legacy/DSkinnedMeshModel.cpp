#include "DSkinnedMeshModel.h"
#include "GeometryGenerator.h"
#include "AnimHelper.h"

namespace hlab {

    using std::make_shared;
    // DModel에서는 Mesh Loading
    // DSkinnedMeshModel에서는 Animation Loading
    DSkinnedMeshModel::DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename)
        {
            m_boneTransforms = make_shared< StructuredBuffer<Matrix>>();
            Initialize(device, context, basePath, filename);
        }
        DSkinnedMeshModel::DSkinnedMeshModel(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename,
            const string& animPath,
            const string& animFilename)
        {
            Initialize(device, context, basePath, filename);
        }
        void DSkinnedMeshModel::Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath, const string& filename)
        {
            DModel::Initialize(device, context, basePath, filename);
        }
         
        void DSkinnedMeshModel::InitMeshBuffers(ComPtr<ID3D11Device>& device, const MeshData& meshData,
            shared_ptr<Mesh>& newMesh) {
            D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                newMesh->vertexBuffer);
            newMesh->indexCount = UINT(meshData.indices.size());
            newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
            newMesh->stride = UINT(sizeof(SkinnedVertex));
            D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                newMesh->indexBuffer);
        }

        void DSkinnedMeshModel::UpdateAnimation(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            string clipId, int frame, int type) {
            //AnimHelper::GetInstance().UpdateAnimation(this, clipId, frame);
        }

        void DSkinnedMeshModel::Render(ComPtr<ID3D11DeviceContext>& context) {

            // ConstBuffer 대신 StructuredBuffer 사용
            // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());
            context->VSSetShaderResources(
                9, 1, m_boneTransforms->GetAddressOfSRV()); // 항상 slot index 주의
            
            // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
            // Model::Render(.)를 같이 사용 가능

            DModel::Render(context);
        };

        void DSkinnedMeshModel::IntegrateRootTransformToWorldTransform(ComPtr<ID3D11DeviceContext>& context)
        {
            Vector3 RootMotionTransition = m_accumulatedRootTransformToLocal.Translation();
            RootMotionTransition.y = 0;
            UpdateWorldRow(Matrix::CreateTranslation(RootMotionTransition) * m_worldRow);

            auto temp = m_accumulatedRootTransform.Translation();
            temp.x = 0.0;
            temp.z = 0.0;
            m_accumulatedRootTransform.Translation(temp);
            UpdateConstantBuffers(context);
        }


} // namespace hlab