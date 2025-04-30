#include "DSkinnedMeshModel2.h"
#include "../D3D12Core/Engine.h"
#include "../D3D12Core/EnginePch.h"
#include "../D3D12Core/Device.h"
#include "../D3D12Core/D3D12Utils.h"
#include "nvtx3/nvToolsExt.h"
namespace dengine {
using std::make_shared;
DSkinnedMeshModel::DSkinnedMeshModel(const string& basePath, const string& filename)
{
    m_boneTransforms = std::make_shared<StructuredBuffer<Matrix>>();
    Initialize(basePath, filename);
}

void DSkinnedMeshModel::Tick(float dt)
{
    DModel::Tick(dt);
}
void DSkinnedMeshModel::Render()
{
    m_boneTransforms->PushGraphicsData();
    GEngine->GetGraphicsDescHeap()->SetGraphicsRootDescriptorTable(5, SRV_REGISTER::t9);
    DModel::Render();
};

void DSkinnedMeshModel::UploadBuffers()
{
    DModel::UploadBuffers();
    if (m_initializeMesh == false)
    {
        return;
    }
    nvtxRangePushA("m_boneTransformsUpload");
    m_boneTransforms->Upload();
    nvtxRangePop();
}

void DSkinnedMeshModel::IntegrateRootTransformToWorldTransform()
{
    Vector3 RootMotionTransition = m_accumulatedRootTransformToLocal.Translation();
    RootMotionTransition.y = 0;
    UpdateWorldRow(Matrix::CreateTranslation(RootMotionTransition) * m_worldRow);

    auto temp = m_accumulatedRootTransform.Translation();
    temp.x = 0.0;
    temp.z = 0.0;
    m_accumulatedRootTransform.Translation(temp);
}


} // namespace dengine