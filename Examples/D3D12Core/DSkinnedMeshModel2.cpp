#include "DSkinnedMeshModel2.h"
#include "Engine.h"
#include "EnginePch.h"
#include "Device.h"
#include "D3D12Utils.h"
namespace hlab {

    using std::make_shared;
    DSkinnedMeshModel2::DSkinnedMeshModel2(const string& basePath, const string& filename)
    {
        m_boneTransforms = std::make_shared<StructuredBuffer2<Matrix>>();
        m_boneTransforms->Init(); 
        Initialize(basePath, filename);
    }

    void DSkinnedMeshModel2::InitMeshBuffers(const MeshData& meshData,
        shared_ptr<DMesh>& newMesh) 
    {
        D3D12Utils::CreateVertexBuffer(DEVICE, meshData.skinnedVertices,
            newMesh->vertexBuffer, newMesh->vertexBufferView);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
        newMesh->stride = UINT(sizeof(SkinnedVertex));
        D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
            newMesh->indexBuffer, newMesh->indexBufferView);
    }

    void DSkinnedMeshModel2::Render() 
    {
        //m_boneTransforms->PushGraphicsData(CBV_REGISTER::b3);
        DModel2::Render();
    };

    void DSkinnedMeshModel2::IntegrateRootTransformToWorldTransform()
    {
        Vector3 RootMotionTransition = m_accumulatedRootTransformToLocal.Translation();
        RootMotionTransition.y = 0;
        UpdateWorldRow(Matrix::CreateTranslation(RootMotionTransition) * m_worldRow);

        auto temp = m_accumulatedRootTransform.Translation();
        temp.x = 0.0;
        temp.z = 0.0;
        m_accumulatedRootTransform.Translation(temp);
        UpdateConstantBuffers();
    }


} // namespace hlab