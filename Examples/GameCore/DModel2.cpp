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

    if (MeshLoadHelper::LoadModel(basePath, filename))
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
void DModel::UploadBuffers()
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
    UploadBuffers();
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
 
void DModel::Render()
{ 
    if (m_initializeMesh == false)
    {
        return;
    }
    if (m_isVisible) 
    {
        if (GEngine->GetPSOType() == PSOType::DEFAULT
            || GEngine->GetPSOType() == PSOType::SHADOW)
        {
            m_meshConsts.PushGraphicsData();
            m_materialConsts.PushGraphicsData();
            GEngine->GetGraphicsDescHeap()->SetGraphicsRootDescriptorTable(3, CBV_REGISTER::b1);
        }
        
        for (auto& mesh : *m_meshes)
        {
            for (int i = int(SRV_REGISTER::t0); i <= int(SRV_REGISTER::t5); i++)
            {
                GEngine->GetGraphicsDescHeap()->ClearSRV(SRV_REGISTER(i));
            }

            if (GEngine->GetPSOType() == PSOType::DEFAULT
                    || GEngine->GetPSOType() == PSOType::SHADOW)
            {
                if (mesh.heightTexture != nullptr)
                {
                    mesh.heightTexture->PushGraphicsData();
                }

                if (mesh.albedoTexture != nullptr)
                {
                    mesh.albedoTexture->PushGraphicsData();
                }

                if (mesh.normalTexture != nullptr)
                {
                    mesh.normalTexture->PushGraphicsData();
                }

                if (mesh.aoTexture != nullptr)
                {
                    mesh.aoTexture->PushGraphicsData();
                }

                if (mesh.metallicRoughnessTexture != nullptr)
                {
                    mesh.metallicRoughnessTexture->PushGraphicsData();
                }

                if (mesh.emissiveTexture != nullptr)
                {
                    mesh.emissiveTexture->PushGraphicsData();
                }
                GEngine->GetGraphicsDescHeap()->SetGraphicsRootDescriptorTable(4, SRV_REGISTER::t0);
            }
            GEngine->GetGraphicsDescHeap()->CommitTable();
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

void DModel::SetMaterialConstants(const MaterialConstants& inMeshConstants)
{
    m_materialConsts.GetCpu() = inMeshConstants;
}

bool DModel::LoadMesh()
{
    if (m_initializeMesh)
    {
        return true;
    }

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