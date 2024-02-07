#include "MeshLoadHelper.h"
#include "Actor.h"
#include "ThreadPool.h"
#include "GeometryGenerator.h"
#include "ConstantBuffers.h"


#include <filesystem>
namespace hlab {
    using namespace DirectX;
    BoundingBox GetBoundingBox(const vector<hlab::Vertex>& vertices) {

        if (vertices.size() == 0)
            return BoundingBox();

        Vector3 minCorner = vertices[0].position;
        Vector3 maxCorner = vertices[0].position;

        for (size_t i = 1; i < vertices.size(); i++) {
            minCorner = Vector3::Min(minCorner, vertices[i].position);
            maxCorner = Vector3::Max(maxCorner, vertices[i].position);
        }

        Vector3 center = (minCorner + maxCorner) * 0.5f;
        Vector3 extents = maxCorner - center;

        return BoundingBox(center, extents);
    }

    void ExtendBoundingBox(const BoundingBox& inBox, BoundingBox& outBox) {

        Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
        Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

        minCorner = Vector3::Min(minCorner,
            Vector3(outBox.Center) - Vector3(outBox.Extents));
        maxCorner = Vector3::Max(maxCorner,
            Vector3(outBox.Center) + Vector3(outBox.Extents));

        outBox.Center = (minCorner + maxCorner) * 0.5f;
        outBox.Extents = maxCorner - outBox.Center;
    }

vector<Mesh> CreateMeshData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, string path, string name, 
    MeshBlock& OutMeshBlock)
{
	auto [mesheDatas, _] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
    vector<Mesh> meshes;
    for (const auto& meshData : mesheDatas) {
        auto newMesh = Mesh();

        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            newMesh.vertexBuffer);
        newMesh.indexCount = UINT(meshData.indices.size());
        newMesh.vertexCount = UINT(meshData.vertices.size());
        newMesh.stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
            newMesh.indexBuffer);

        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        newMesh.albedoTexture, newMesh.albedoSRV);
                }
                else {
                    D3D11Utils::CreateTexture(
                        device, context, meshData.albedoTextureFilename, true,
                        newMesh.albedoTexture, newMesh.albedoSRV);
                }
                OutMeshBlock.useAlbedoMap = true;
            }
            else {
                cout << meshData.albedoTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            if (filesystem::exists(meshData.emissiveTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.emissiveTextureFilename, true,
                    newMesh.emissiveTexture, newMesh.emissiveSRV);
                OutMeshBlock.useEmissiveMap = true;
            }
            else {
                cout << meshData.emissiveTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.normalTextureFilename, false,
                    newMesh.normalTexture, newMesh.normalSRV);
                OutMeshBlock.useNormalMap = true;
            }
            else {
                cout << meshData.normalTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                D3D11Utils::CreateTexture(
                    device, context, meshData.heightTextureFilename, false,
                    newMesh.heightTexture, newMesh.heightSRV);
                OutMeshBlock.useHeightMap = true;
            }
            else {
                cout << meshData.heightTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                D3D11Utils::CreateTexture(device, context,
                    meshData.aoTextureFilename, false,
                    newMesh.aoTexture, newMesh.aoSRV);
                OutMeshBlock.useAOMap = true;
            }
            else {
                cout << meshData.aoTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {

            if (filesystem::exists(meshData.metallicTextureFilename) &&
                filesystem::exists(meshData.roughnessTextureFilename)) {

                D3D11Utils::CreateMetallicRoughnessTexture(
                    device, context, meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    newMesh.metallicRoughnessTexture,
                    newMesh.metallicRoughnessSRV);
            }
            else {
                cout << meshData.metallicTextureFilename << " or "
                    << meshData.roughnessTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.metallicTextureFilename.empty()) {
            OutMeshBlock.useMetalicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            OutMeshBlock.useRoughnessMap = true;
        }
        meshes.push_back(newMesh);
    }
    // Initialize Bounding Box
    {
        OutMeshBlock.boundingBox = GetBoundingBox(mesheDatas[0].vertices);
        for (size_t i = 1; i < mesheDatas.size(); i++) {
            auto bb = GetBoundingBox(mesheDatas[0].vertices);
            ExtendBoundingBox(bb, OutMeshBlock.boundingBox);
        }
        auto meshData = GeometryGenerator::MakeWireBox(
            OutMeshBlock.boundingBox.Center,
            Vector3(OutMeshBlock.boundingBox.Extents) + Vector3(1e-3f));
        OutMeshBlock.boundingBoxMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            OutMeshBlock.boundingBoxMesh->vertexBuffer);
        OutMeshBlock.boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        OutMeshBlock.boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        OutMeshBlock.boundingBoxMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
            OutMeshBlock.boundingBoxMesh->indexBuffer);
        }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto& mesh : mesheDatas) {
            for (auto& v : mesh.vertices) {
                maxRadius = std::max(
                    (Vector3(OutMeshBlock.boundingBox.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // 살짝 크게 설정
        OutMeshBlock.boundingSphere = BoundingSphere(OutMeshBlock.boundingBox.Center, maxRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            OutMeshBlock.boundingSphere.Center, OutMeshBlock.boundingSphere.Radius);
        OutMeshBlock.boundingSphere = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            OutMeshBlock.boundingSphereMesh->vertexBuffer);
        OutMeshBlock.boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        OutMeshBlock.boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        OutMeshBlock.boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
            OutMeshBlock.boundingSphereMesh->indexBuffer);
    }
    

	return meshes;
}
AnimationData ReadAnimationFromFile(string path, string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool MeshLoadHelper::LoadModelData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const string& InPath, const string& InName, vector<Mesh>* OutMeshes)
{
	string key = InPath + InName;
	if (MeshMap.find(key) == MeshMap.end())
	{
		MeshMap[key] = MeshBlock();

		ThreadPool& tPool =  ThreadPool::getInstance();
		MeshMap[key].Loader = tPool.EnqueueJob(CreateMeshData, device, context, InPath, InName, MeshMap[key]);
		MeshMap[key].IsLoading = true;
		return false;
	}
	if (MeshMap[key].IsLoading ==true && MeshMap[key].Loader._Is_ready() == false)
	{
		return false;
	}
	if (MeshMap[key].IsLoading == true)
	{
		MeshMap[key].IsLoading = false;
		MeshMap[key].Meshes = MeshMap[key].Loader.get();
	}
	OutMeshes = &(MeshMap[key].Meshes);
	return true;
}

bool MeshLoadHelper::GetMesh(const string& InPath, const string& InName, vector<Mesh>* OutMesh)
{
    string key = InPath + InName;
    if (MeshMap.find(key) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[key].IsLoading == true && MeshMap[key].Loader._Is_ready() == false)
    {
        return false;
    }
    if (MeshMap[key].IsLoading == true)
    {
        MeshMap[key].IsLoading = false;
        MeshMap[key].Meshes = MeshMap[key].Loader.get();
    }
    OutMesh = &(MeshMap[key].Meshes);
    return true;
}
}