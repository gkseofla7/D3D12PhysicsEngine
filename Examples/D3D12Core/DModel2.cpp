#include "DModel2.h"
#include "D3D12Utils.h"
//#include "GeometryGenerator.h"
#include "MeshLoadHelper2.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
#include "MeshLoadHelper2.h"
#include <filesystem>

namespace hlab {

using namespace std;
using namespace DirectX;
DModel2::DModel2(const string& basePath, const string& filename)
{
    Initialize(basePath, filename);
}
DModel2::DModel2(const string& meshKey)
{
    Initialize(meshKey);
}

void DModel2::InitMeshBuffers(const MeshData& meshData,
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

void DModel2::Initialize(const std::string& basePath,
    const std::string& filename) {
    // �Ϲ������δ� DMesh���� m_mesh/materialConsts�� ���� ���� ����
    // ���⼭�� �� Model ���� ���� DMesh���� Consts�� ��� ����
    m_basePath = basePath;
    m_filename = filename;

    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Init(CBV_REGISTER::b1, FRAMEBUFFER_COUNT);
    m_materialConsts.Init(CBV_REGISTER::b2, FRAMEBUFFER_COUNT);

    if (MeshLoadHelper2::LoadModelData(basePath, filename))
    {
        MeshLoadHelper2::GetMaterial(m_basePath, m_filename, m_materialConsts.GetCpu());
    }
}
void DModel2::Initialize(const string& meshKey) 
{
    m_meshKey = meshKey;

    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Init(CBV_REGISTER::b1, FRAMEBUFFER_COUNT);
    m_materialConsts.Init(CBV_REGISTER::b2, FRAMEBUFFER_COUNT);
}
void DModel2::UpdateConstantBuffers() 
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
void DModel2::Tick(float dt)
{
    if (m_initializeMesh == false)
    {
        m_initializeMesh = LoadMesh();
        if (m_initializeMesh == false)
        {
            return;
        }
    }
}
void DModel2::UpdatePosition(const Vector3& inDelta)
{
    Matrix newMatrix = Matrix::CreateTranslation(inDelta) * m_worldRow;
    UpdateWorldRow(newMatrix);
}
void DModel2::SetWorldPosition(const Vector3& InPos)
{
    m_worldRow.Translation(InPos);
    UpdateWorldRow(m_worldRow);
}

void DModel2::UpdateRotation(const Matrix& inDelta)
{
    m_direction = Vector3::Transform(m_direction, inDelta);
    m_direction.Normalize();

    Vector3 ModelPos = m_worldRow.Translation();
    m_worldRow.Translation(Vector3(0.0f));
    Matrix newMatrix = m_worldRow * inDelta;
    newMatrix.Translation(ModelPos);
    UpdateWorldRow(newMatrix);
}
void DModel2::SetDirection(const Vector3& inDirection)
{
    Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
    dir = Vector4::Transform(
        dir, m_worldRow);
    dir.Normalize();
    Vector3 dir3 = Vector3(dir.x, dir.y, dir.z);
    float theta = acos(dir3.Dot(inDirection) / (dir3.Length() * inDirection.Length()));
    UpdateRotation(Matrix::CreateRotationY(theta));
}
//GraphicsPSO2& DModel2::GetPSO() {
//    return Graphics::defaultSolidPSO;
//}

//GraphicsPSO2& DModel2::GetDepthOnlyPSO() { return Graphics::depthOnlyPSO; }
//
//GraphicsPSO2& DModel2::GetReflectPSO(const bool wired) {
//    return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
//}

void DModel2::Render() {
    if (m_initializeMesh == false)
    {
        return;
    }
    if (m_isVisible) {

        for (auto& mesh : *m_meshes) 
        {
            m_meshConsts.PushGraphicsData();
            m_materialConsts.PushGraphicsData();

            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.heightTexture->GetSRVHandle(), SRV_REGISTER::t0);
            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.albedoTexture->GetSRVHandle(), SRV_REGISTER::t1);
            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.normalTexture->GetSRVHandle(), SRV_REGISTER::t2);
            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.aoTexture->GetSRVHandle(), SRV_REGISTER::t3);
            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.metallicRoughnessTexture->GetSRVHandle(), SRV_REGISTER::t4);
            GEngine->GetGraphicsDescHeap()->SetSRV(mesh.emissiveTexture->GetSRVHandle(), SRV_REGISTER::t5);

            GRAPHICS_CMD_LIST->IASetVertexBuffers(0, 1, &mesh.vertexBufferView); // Slot: (0~15)
            GRAPHICS_CMD_LIST->IASetIndexBuffer(&mesh.indexBufferView);
            GRAPHICS_CMD_LIST->DrawIndexedInstanced(mesh.indexCount, 1, 0, 0, 0);

            GEngine->GetGraphicsDescHeap()->CommitTable();
            // TODO. release �ʿ�?
        }
    }
}

void DModel2::UpdateAnimation(string clipId, int frame, int type) {
    // class skinnedMeshModel���� override
    cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
        "int clipId, int frame) was not implemented."
        << endl;
    exit(-1);
}


void DModel2::UpdateWorldRow(const Matrix& worldRow) {
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

bool DModel2::LoadMesh()
{
    if (m_initializeMesh)
    {
        return true;
    }
    // TODO. UpdateConstantBuffers�� �ƴ� ���� Update �Լ��� �����°� ����
    if (m_meshKey.size() == 0)
    {
        m_meshKey = m_basePath + m_filename;
    }

    if (m_initializeMesh = MeshLoadHelper2::GetMesh(m_meshKey, m_meshes))
    {
        MeshLoadHelper2::GetBoundingMesh(m_meshKey, m_boundingSphere, m_boundingBox, m_boundingSphereMesh, m_boundingBoxMesh);
        MeshLoadHelper2::GetMaterial(m_meshKey, m_materialConsts.GetCpu());

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
} // namespace hlab