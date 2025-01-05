#include "DSkinnedMeshModel2.h"
#include "../D3D12Core/Engine.h"
#include "../D3D12Core/EnginePch.h"
#include "../D3D12Core/Device.h"
#include "../D3D12Core/D3D12Utils.h"
namespace dengine {
using std::make_shared;
DSkinnedMeshModel::DSkinnedMeshModel(const string& basePath, const string& filename)
{
    m_boneTransforms = std::make_shared<StructuredBuffer<Matrix>>();
    Initialize(basePath, filename);
}

void DSkinnedMeshModel::InitMeshBuffers(const MeshData& meshData,
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

void DSkinnedMeshModel::Render()
{
    m_boneTransforms->PushGraphicsData(SRV_REGISTER::t9);
    DModel::Render();
};

void DSkinnedMeshModel::IntegrateRootTransformToWorldTransform()
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


} // namespace dengine