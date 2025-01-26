
#include "DModel.h"
#include "GeometryGenerator.h"
#include "MeshLoadHelper.h"

#include <filesystem>

namespace hlab {

    using namespace std;
    using namespace DirectX;


    DModel::DModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const std::string& basePath, const std::string& filename) {
        Initialize(device, context, basePath, filename);
    }
    DModel::DModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const string& meshKey)
    {
        Initialize(device, context, meshKey);
    }

    void DModel::InitMeshBuffers(ComPtr<ID3D11Device>& device,
        const MeshData& meshData,
        shared_ptr<Mesh>& newMesh) {

        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.vertices.size());
        newMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
            newMesh->indexBuffer);
    }

    void DModel::Initialize(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const std::string& basePath,
        const std::string& filename) {
        // �Ϲ������δ� Mesh���� m_mesh/materialConsts�� ���� ���� ����
        // ���⼭�� �� Model ���� ���� Mesh���� Consts�� ��� ����
        m_basePath = basePath;
        m_filename = filename;

        m_meshConsts.GetCpu().world = Matrix();
        m_meshConsts.Initialize(device);
        m_materialConsts.Initialize(device);

        if (MeshLoadHelper::LoadModelData(basePath, filename))
        {
            MeshLoadHelper::GetMaterial(m_basePath, m_filename, m_materialConsts.GetCpu());
        }
    }
    void DModel::Initialize(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const string& meshKey) {
        m_meshKey = meshKey;

        m_meshConsts.GetCpu().world = Matrix();
        m_meshConsts.Initialize(device);
        m_materialConsts.Initialize(device);
    }
    void DModel::UpdateConstantBuffers(ComPtr<ID3D11DeviceContext>& context) {
        if (m_initializeMesh == false)
        {
            return;
        }
        if (m_isVisible) {
            m_meshConsts.Upload(context);
            m_materialConsts.Upload(context);
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
    }
    void DModel::UpdatePosition(const Vector3& inDelta)
    {
        Matrix newMatrix = Matrix::CreateTranslation(inDelta)* m_worldRow;
        UpdateWorldRow(newMatrix);
    }
    void DModel::SetWorldPosition(const Vector3& InPos)
    {
        m_worldRow.Translation(InPos);
        UpdateWorldRow(m_worldRow);
    }

    void DModel::UpdateRotation(const Matrix& inDelta)
    {
        m_direction =Vector3::Transform( m_direction,inDelta);
        m_direction.Normalize();

        Vector3 ModelPos = m_worldRow.Translation();
        m_worldRow.Translation(Vector3(0.0f));
        Matrix newMatrix = m_worldRow*inDelta;
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
    GraphicsPSO& DModel::GetPSO(const bool wired) {
        return wired ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO;
    }

    GraphicsPSO& DModel::GetDepthOnlyPSO() { return Graphics::depthOnlyPSO; }

    GraphicsPSO& DModel::GetReflectPSO(const bool wired) {
        return wired ? Graphics::reflectWirePSO : Graphics::reflectSolidPSO;
    }

    void DModel::Render(ComPtr<ID3D11DeviceContext>& context) {
        if (m_initializeMesh == false)
        {
            return;
        } 
        if (m_isVisible) { 
            for (auto& mesh : *m_meshes) {
                //TODO mesh Consts�� ������, Model �������� ���
                ID3D11Buffer* constBuffers[2] = { m_meshConsts.Get(),
                                                 m_materialConsts.Get() };
                context->VSSetConstantBuffers(1, 2, constBuffers);

                context->VSSetShaderResources(0, 1, mesh.heightSRV.GetAddressOf());

                // ��ü �������� �� �������� �ؽ��� ��� (t0 ���ͽ���)
                vector<ID3D11ShaderResourceView*> resViews = {
                    mesh.albedoSRV.Get(), mesh.normalSRV.Get(), mesh.aoSRV.Get(),
                    mesh.metallicRoughnessSRV.Get(), mesh.emissiveSRV.Get() };
                context->PSSetShaderResources(0, // register(t0)
                    UINT(resViews.size()),
                    resViews.data());
                context->PSSetConstantBuffers(1, 2, constBuffers);

                // Volume Rendering
                if (mesh.densityTex.GetSRV())
                    context->PSSetShaderResources(
                        5, 1, mesh.densityTex.GetAddressOfSRV());
                if (mesh.lightingTex.GetSRV())
                    context->PSSetShaderResources(
                        6, 1, mesh.lightingTex.GetAddressOfSRV());

                context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(),
                    &mesh.stride, &mesh.offset);
                context->IASetIndexBuffer(mesh.indexBuffer.Get(),
                    DXGI_FORMAT_R32_UINT, 0);
                context->DrawIndexed(mesh.indexCount, 0, 0);

                // Release resources
                ID3D11ShaderResourceView* nulls[3] = { NULL, NULL, NULL };
                context->PSSetShaderResources(5, 3, nulls);
            }
        }
    }

    void DModel::UpdateAnimation(ComPtr<ID3D11DeviceContext>& context, string clipId,
        int frame, int type = 0) {
        // class skinnedMeshModel���� override
        cout << "Model::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, "
            "int clipId, int frame) was not implemented."
            << endl;
        exit(-1);
    }

    void DModel::RenderWireBoundingBox(ComPtr<ID3D11DeviceContext>& context) {
        if (m_initializeMesh == false)
        {
            return;
        }
        ID3D11Buffer* constBuffers[2] = {
            m_boundingBoxMesh->meshConstsGPU.Get(),
            m_boundingBoxMesh->materialConstsGPU.Get() };
        context->VSSetConstantBuffers(1, 2, constBuffers);
        context->IASetVertexBuffers(
            0, 1, m_boundingBoxMesh->vertexBuffer.GetAddressOf(),
            &m_boundingBoxMesh->stride, &m_boundingBoxMesh->offset);
        context->IASetIndexBuffer(m_boundingBoxMesh->indexBuffer.Get(),
            DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(m_boundingBoxMesh->indexCount, 0, 0);
    }

    void DModel::RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>& context) {
        if (m_initializeMesh == false)
        {
            return;
        }
        m_boundingSphereMesh->meshConstsGPU = m_meshConsts.Get();
        m_boundingSphereMesh->materialConstsGPU = m_materialConsts.Get();

        ID3D11Buffer* constBuffers[2] = {
            m_boundingSphereMesh->meshConstsGPU.Get(),
            m_boundingSphereMesh->materialConstsGPU.Get() };
        context->VSSetConstantBuffers(1, 2, constBuffers);
        context->IASetVertexBuffers(
            0, 1, m_boundingSphereMesh->vertexBuffer.GetAddressOf(),
            &m_boundingSphereMesh->stride, &m_boundingSphereMesh->offset);
        context->IASetIndexBuffer(m_boundingSphereMesh->indexBuffer.Get(),
            DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(m_boundingSphereMesh->indexCount, 0, 0);
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
        // TODO. UpdateConstantBuffers�� �ƴ� ���� Update �Լ��� �����°� ����
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

            m_boundingBox.Extents = XMFLOAT3(m_boundingBox.Extents.x*m_scale, m_boundingBox.Extents.y * m_scale, m_boundingBox.Extents.z * m_scale);
        }
        else
        {
            return false;
        }
        return true;
    }
} // namespace hlab