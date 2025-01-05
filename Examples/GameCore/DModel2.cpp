#include "DModel2.h"
#include "../D3D12Core/D3D12Utils.h"
//#include "GeometryGenerator.h"
#include "MeshLoadHelper2.h"
#include "../D3D12Core/Engine.h"
#include "../D3D12Core/Device.h"
#include "../D3D12Core/CommandQueue.h"
#include <filesystem>

namespace dengine {

using namespace std;
using namespace DirectX;
DModel::DModel(const string& basePath, const string& filename)
{
    Initialize(basePath, filename);
}
DModel::DModel(const string& meshKey)
{
    Initialize(meshKey);
}

void DModel::InitMeshBuffers(const MeshData& meshData,
    shared_ptr<DMesh>& newMesh)
{
    D3D12Utils::CreateVertexBuffer(DEVICE, meshData.vertices,
        newMesh->vertexBuffer, newMesh->vertexBufferView);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
        newMesh->indexBuffer, newMesh->indexBufferView);
}

void DModel::Initialize(const std::string& basePath,
    const std::string& filename) {
    // 일반적으로는 DMesh들이 m_mesh/materialConsts를 각자 소유 가능
    // 여기서는 한 Model 안의 여러 DMesh들이 Consts를 모두 공유
    m_basePath = basePath;
    m_filename = filename;

    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Init(CBV_REGISTER::b1, SWAP_CHAIN_BUFFER_COUNT);
    m_materialConsts.Init(CBV_REGISTER::b2, SWAP_CHAIN_BUFFER_COUNT);

    if (MeshLoadHelper::LoadModelData(basePath, filename))
    {
        MeshLoadHelper::GetMaterial(m_basePath, m_filename, m_materialConsts.GetCpu());
    }
}
void DModel::Initialize(const string& meshKey) 
{
    m_meshKey = meshKey;

    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Init(CBV_REGISTER::b1, SWAP_CHAIN_BUFFER_COUNT);
    m_materialConsts.Init(CBV_REGISTER::b2, SWAP_CHAIN_BUFFER_COUNT);
}
void DModel::UpdateConstantBuffers() 
{
    if (m_initializeMesh == false)
    {
        return;
    }
    if (m_isVisible) { 
        m_meshConsts.Upload();
        m_materialConsts.Upload();
    }
}
void DModel::Tick(float dt)
{
    if (m_initializeMesh == false)
    {
        m_initializeMesh = LoadMesh();
        if (m_initializeMesh == false)
        {
            return;
        }
    }
    UpdateConstantBuffers();
}
void DModel::UpdatePosition(const Vector3& inDelta) 
{
    Matrix newMatrix = Matrix::CreateTranslation(inDelta) * m_worldRow;
    UpdateWorldRow(newMatrix);
}
void DModel::SetWorldPosition(const Vector3& InPos)
{
    m_worldRow.Translation(InPos);
    UpdateWorldRow(m_worldRow);
}

void DModel::UpdateRotation(const Matrix& inDelta)
{
    m_direction = Vector3::Transform(m_direction, inDelta);
    m_direction.Normalize();

    Vector3 ModelPos = m_worldRow.Translation();
    m_worldRow.Translation(Vector3(0.0f));
    Matrix newMatrix = m_worldRow * inDelta;
    newMatrix.Translation(ModelPos);
    UpdateWorldRow(newMatrix);
}
void DModel::SetDirection(const Vector3& inDirection)
{
    Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
    dir = Vector4::Transform(
        dir, m_worldRow);
    dir.Normalize();
    Vector3 dir3 = Vector3(dir.x, dir.y, dir.z);
    float theta = acos(dir3.Dot(inDirection) / (dir3.Length() * inDirection.Length()));
    UpdateRotation(Matrix::CreateRotationY(theta));
}
//GraphicsPSO2& DModel::GetPSO() {
//    return Graphics::defaultSolidPSO;
//}

//GraphicsPSO2& DModel::GetDepthOnlyPSO() { return Graphics::depthOnlyPSO; }
//
//GraphicsPSO2& DModel::GetReflectPSO(const bool wired) {
//    return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
//}
 
void DModel::Render()
{ 
    if (m_initializeMesh == false)
    {
        return;
    }
    if (m_isVisible) 
    {
        
        for (auto& mesh : *m_meshes) 
        {
            if (GEngine->GetPSOType() == PSOType::DEFAULT
                    || GEngine->GetPSOType() == PSOType::SHADOW)
            {
                m_meshConsts.PushGraphicsData();
                m_materialConsts.PushGraphicsData();

                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t0);
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t1);
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t2);
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t3);
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t4);
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER::t5);

                if (mesh.heightTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.heightTexture->GetSRVHandle(), SRV_REGISTER::t0);
                }
                if (mesh.albedoTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.albedoTexture->GetSRVHandle(), SRV_REGISTER::t1);
                }
                if (mesh.normalTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.normalTexture->GetSRVHandle(), SRV_REGISTER::t2);
                }
                if (mesh.aoTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.aoTexture->GetSRVHandle(), SRV_REGISTER::t3);
                }
                if (mesh.metallicRoughnessTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.metallicRoughnessTexture->GetSRVHandle(), SRV_REGISTER::t4);
                }
                if (mesh.emissiveTexture != nullptr)
                {
                    GEngine->GetGraphicsDescHeap()->SetSRV(mesh.emissiveTexture->GetSRVHandle(), SRV_REGISTER::t5);
                }
                GEngine->GetGraphicsDescHeap()->CommitTable();
            }
            
            GRAPHICS_CMD_LIST->IASetVertexBuffers(0, 1, &mesh.vertexBufferView); // Slot: (0~15)
            GRAPHICS_CMD_LIST->IASetIndexBuffer(&mesh.indexBufferView);
            GRAPHICS_CMD_LIST->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);
        }
    }
}

void DModel::UpdateAnimation(string clipId, int frame, int type) {
    // class skinnedMeshModel에서 override
    cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
        "int clipId, int frame) was not implemented."
        << endl;
    exit(-1);
}


void DModel::UpdateWorldRow(const Matrix& worldRow) {
    this->m_worldRow = worldRow;
    this->m_worldITRow = worldRow;
    m_worldITRow.Translation(Vector3(0.0f));
    m_worldITRow = m_worldITRow.Invert().Transpose();

    if (m_initializeMesh)
    {
        m_boundingSphere.Center = this->m_worldRow.Translation();
    }

    m_meshConsts.GetCpu().world = worldRow.Transpose();
    m_meshConsts.GetCpu().worldIT = m_worldITRow.Transpose();
    m_meshConsts.GetCpu().worldInv = m_meshConsts.GetCpu().world.Invert();
}

bool DModel::LoadMesh()
{
    if (m_initializeMesh)
    {
        return true;
    }
    // TODO. UpdateConstantBuffers가 아닌 따로 Update 함수로 빼내는게 좋다
    if (m_meshKey.size() == 0)
    {
        m_meshKey = m_basePath + m_filename;
    }

    if (m_initializeMesh = MeshLoadHelper::GetMesh(m_meshKey, m_meshes))
    {
        MeshLoadHelper::GetBoundingMesh(m_meshKey, m_boundingSphere, m_boundingBox, m_boundingSphereMesh, m_boundingBoxMesh);
        MeshLoadHelper::GetMaterial(m_meshKey, m_materialConsts.GetCpu());

        //m_boundingSphereMesh->meshConstsGPU = m_meshConsts.Get();
        //m_boundingSphereMesh->materialConstsGPU = m_materialConsts.Get();

        //m_boundingBoxMesh->meshConstsGPU = m_meshConsts.Get();
        //m_boundingBoxMesh->materialConstsGPU = m_materialConsts.Get();

        m_boundingSphere.Radius = m_scale * m_boundingSphere.Radius;

        m_boundingBox.Extents = XMFLOAT3(m_boundingBox.Extents.x * m_scale, m_boundingBox.Extents.y * m_scale, m_boundingBox.Extents.z * m_scale);
    }
    else
    {
        return false;
    }
    return true;
}
} // namespace dengine