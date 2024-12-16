#include "MeshLoadHelper2.h"
#include "Engine.h"
#include "Device.h"
#include "D3D12Utils.h"
#include "GeometryGenerator2.h"
#include "../ThreadPool.h"
#include <filesystem>
#include "Mesh2.h"

namespace dengine {
    using namespace DirectX;
    map<string, MeshBlock> MeshLoadHelper::MeshMap;
    std::mutex MeshLoadHelper::m_mtx;
    BoundingBox GetBoundingBoxFromVertices2(const vector<dengine::Vertex>& vertices) {

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

    void GetExtendBoundingBox2(const BoundingBox& inBox, BoundingBox& outBox) {

        Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
        Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

        minCorner = Vector3::Min(minCorner,
            Vector3(outBox.Center) - Vector3(outBox.Extents));
        maxCorner = Vector3::Max(maxCorner,
            Vector3(outBox.Center) + Vector3(outBox.Extents));

        outBox.Center = (minCorner + maxCorner) * 0.5f;
        outBox.Extents = maxCorner - outBox.Center;
    }

vector<dengine::MeshData> CreateMeshData(MeshBlock& OutMeshBlock)
{ 
	auto [mesheDatas, _] =
		GeometryGenerator::ReadAnimationFromFile(OutMeshBlock.PathName, OutMeshBlock.FileName);
    return mesheDatas;
}
AnimationData ReadAnimationFromFile2(string path, string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}

void MeshLoadHelper::LoadAllUnloadedModel()
{
    for (auto& Pair : MeshMap)
    {
        MeshBlock& mBloock = Pair.second;
        if (mBloock.MeshDataLoadType == hlab::ELoadType::Loading && mBloock.Loader._Is_ready() == true)
        {
            hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
            //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
            auto func = [&Pair]() {
                return LoadModel(Pair.first); };
            tPool.EnqueueJob(func);
        } 
    }
}
bool MeshLoadHelper::LoadModelData( const string& inPath, const string& inName)
{
	string key = inPath + inName;
	if (MeshMap.find(key) == MeshMap.end())
	{
		MeshMap[key] = MeshBlock();
         
        MeshBlock& meshBlocks = MeshMap[key];
        meshBlocks.PathName = inPath;
        meshBlocks.FileName = inName;

        hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
        //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
        auto func = [&meshBlocks]() {
            return CreateMeshData(meshBlocks); };
		MeshMap[key].Loader = tPool.EnqueueJob(func);
		MeshMap[key].MeshDataLoadType = hlab::ELoadType::Loading;
		return false;
	}
	
    return MeshMap[key].MeshDataLoadType == hlab::ELoadType::Loaded;
}
bool MeshLoadHelper::GetMaterial(const string& inPath, const string& inName, MaterialConstants2& InConstants)
{
    string key = inPath + inName;
    return GetMaterial(key, inName, InConstants);
}

bool MeshLoadHelper::GetMaterial(const string& InMeshKey, MaterialConstants2& InConstants)
{
    if (MeshMap.find(InMeshKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InMeshKey].MeshDataLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }
    InConstants.useAlbedoMap = MeshMap[InMeshKey].useAlbedoMap;
    InConstants.useAOMap = MeshMap[InMeshKey].useAOMap;
    InConstants.useEmissiveMap = MeshMap[InMeshKey].useEmissiveMap;
    InConstants.useMetallicMap = MeshMap[InMeshKey].useMetalicMap;
    InConstants.useNormalMap = MeshMap[InMeshKey].useNormalMap;
    InConstants.useRoughnessMap = MeshMap[InMeshKey].useRoughnessMap;
    return true;
}
void MeshLoadHelper::LoadModel(const string& key)
{
    {
        std::lock_guard<std::mutex> lock(MeshLoadHelper::m_mtx);

        if (MeshMap.find(key) == MeshMap.end())
        {
            return;
        }
        // MeshData 로드 안됨
        if (MeshMap[key].MeshDataLoadType == hlab::ELoadType::NotLoaded)
        {
            return;
        }
        // MeshData 로딩중
        if (MeshMap[key].MeshDataLoadType == hlab::ELoadType::Loading && MeshMap[key].Loader._Is_ready() == false)
        {
            return;
        }
        // 이미 모델 로딩중 or Loaded
        if (MeshMap[key].MeshLoadType == hlab::ELoadType::Loading
            || MeshMap[key].MeshLoadType == hlab::ELoadType::Loaded)
        {
            return;
        }
        MeshMap[key].MeshLoadType = hlab::ELoadType::Loading;
    }

    std::vector<MeshData>& meshDatas = MeshMap[key].MeshDatas;
    if (MeshMap[key].MeshDataLoadType == hlab::ELoadType::Loading && MeshMap[key].Loader._Is_ready() == true)
    {
        meshDatas = MeshMap[key].Loader.get();
        MeshMap[key].MeshDataLoadType = hlab::ELoadType::Loaded;
    }

    MeshBlock& meshBlock = MeshMap[key];
    vector<DMesh>& meshes = meshBlock.Meshes;
    int index = 0;
    for (const auto& meshData : meshDatas) { 
        if (meshes.size()<= index)
        {
            meshes.push_back(DMesh());
            
        }
        DMesh& newMesh = meshes[index];
         
        if (meshData.skinnedVertices.size() > 0)
        {
            D3D12Utils::CreateVertexBuffer(DEVICE, meshData.skinnedVertices,
                newMesh.vertexBuffer, newMesh.vertexBufferView);
            newMesh.indexCount = UINT(meshData.indices.size());
            newMesh.vertexCount = UINT(meshData.skinnedVertices.size());
            newMesh.stride = UINT(sizeof(SkinnedVertex));
            D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
                newMesh.indexBuffer, newMesh.indexBufferView);
        }
        else
        {
            D3D12Utils::CreateVertexBuffer(DEVICE, meshData.vertices,
                newMesh.vertexBuffer, newMesh.vertexBufferView);
            newMesh.indexCount = UINT(meshData.indices.size());
            newMesh.vertexCount = UINT(meshData.vertices.size());
            newMesh.stride = UINT(sizeof(Vertex));
            D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
                newMesh.indexBuffer, newMesh.indexBufferView);
        }

        if (!meshData.albedoTextureFilename.empty()) 
        {
            if (filesystem::exists(meshData.albedoTextureFilename)) 
            {
                if (!meshData.opacityTextureFilename.empty()) 
                {
                    ComPtr<ID3D12Resource> tex2D;
                    D3D12Utils::CreateTexture(
                        DEVICE, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        tex2D);
                    newMesh.albedoTexture = std::make_shared<Texture>();
                    newMesh.albedoTexture->CreateFromResource(tex2D);
                }
                else 
                {
                    ComPtr<ID3D12Resource> tex2D;
                    D3D12Utils::CreateTexture(
                        DEVICE, meshData.albedoTextureFilename, true,
                        tex2D);
                    newMesh.albedoTexture = std::make_shared<Texture>();
                    newMesh.albedoTexture->CreateFromResource(tex2D);
                }
                meshBlock.useAlbedoMap = true;
            }
            else 
            {
                cout << meshData.albedoTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty()) 
        {
            if (filesystem::exists(meshData.emissiveTextureFilename)) 
            {
                ComPtr<ID3D12Resource> tex2D;
                D3D12Utils::CreateTexture(DEVICE, meshData.emissiveTextureFilename, true, tex2D);
                newMesh.emissiveTexture = std::make_shared<Texture>();
                newMesh.emissiveTexture->CreateFromResource(tex2D);
                meshBlock.useEmissiveMap = true;
            }
            else 
            {
                cout << meshData.emissiveTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                ComPtr<ID3D12Resource> tex2D;
                D3D12Utils::CreateTexture(DEVICE, meshData.normalTextureFilename, false,tex2D);
                newMesh.normalTexture = std::make_shared<Texture>();
                newMesh.normalTexture->CreateFromResource(tex2D);
                meshBlock.useNormalMap = true;
            }
            else {
                cout << meshData.normalTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                ComPtr<ID3D12Resource> tex2D;
                D3D12Utils::CreateTexture(DEVICE, meshData.heightTextureFilename, false, tex2D);
                newMesh.heightTexture = std::make_shared<Texture>();
                newMesh.heightTexture->CreateFromResource(tex2D);
                meshBlock.useHeightMap = true;
            }
            else {
                cout << meshData.heightTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }
         
        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) 
            {
                ComPtr<ID3D12Resource> tex2D;
                D3D12Utils::CreateTexture(DEVICE, meshData.aoTextureFilename, false, tex2D);
                newMesh.aoTexture = std::make_shared<Texture>();
                newMesh.aoTexture->CreateFromResource(tex2D);
                meshBlock.useAOMap = true;
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
                filesystem::exists(meshData.roughnessTextureFilename)) 
            {
                ComPtr<ID3D12Resource> tex2D;
                D3D12Utils::CreateMetallicRoughnessTexture(
                    DEVICE, meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename, tex2D);
                newMesh.metallicRoughnessTexture = std::make_shared<Texture>();
                newMesh.metallicRoughnessTexture->CreateFromResource(tex2D);
            }
            else {
                cout << meshData.metallicTextureFilename << " or "
                    << meshData.roughnessTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.metallicTextureFilename.empty()) {
            meshBlock.useMetalicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            meshBlock.useRoughnessMap = true;
        }
        index++;
    }

    // Initialize Bounding Box
    {
        meshBlock.boundingBox = GetBoundingBoxFromVertices2(meshDatas[0].vertices);
        for (size_t i = 1; i < meshDatas.size(); i++) {
            auto bb = GetBoundingBoxFromVertices2(meshDatas[0].vertices);
            GetExtendBoundingBox2(bb, meshBlock.boundingBox);
        }

        auto meshData = GeometryGenerator::MakeWireBox(
            meshBlock.boundingBox.Center,
            Vector3(meshBlock.boundingBox.Extents) + Vector3(1e-3f));
        meshBlock.boundingBoxMesh = std::make_shared<DMesh>();

        meshBlock.boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        meshBlock.boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        meshBlock.boundingBoxMesh->stride = UINT(sizeof(Vertex));

        D3D12Utils::CreateVertexBuffer(DEVICE, meshData.vertices,
            meshBlock.boundingBoxMesh->vertexBuffer, meshBlock.boundingBoxMesh->vertexBufferView);
        D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
            meshBlock.boundingBoxMesh->indexBuffer, meshBlock.boundingBoxMesh->indexBufferView);
    }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto& mesh : meshDatas) {
            for (auto& v : mesh.vertices) {
                maxRadius = std::max(
                    (Vector3(meshBlock.boundingBox.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // 살짝 크게 설정
        meshBlock.boundingSphere = BoundingSphere(meshBlock.boundingBox.Center, maxRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            meshBlock.boundingSphere.Center, meshBlock.boundingSphere.Radius);
        meshBlock.boundingSphereMesh = std::make_shared<DMesh>();

        meshBlock.boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        meshBlock.boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        meshBlock.boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D12Utils::CreateVertexBuffer(DEVICE, meshData.vertices,
            meshBlock.boundingSphereMesh->vertexBuffer, meshBlock.boundingSphereMesh->vertexBufferView);
        D3D12Utils::CreateIndexBuffer(DEVICE, meshData.indices,
            meshBlock.boundingSphereMesh->indexBuffer, meshBlock.boundingSphereMesh->indexBufferView);
    }
    MeshMap[key].MeshLoadType = hlab::ELoadType::Loaded;
}


bool MeshLoadHelper::GetMesh(const string& inPath, const string& inName, vector<DMesh>*& OutMesh)
{
    string key = inPath + inName;
    return GetMesh(key, OutMesh);
}

bool MeshLoadHelper::GetMesh(const string& InKey, vector<DMesh>*& OutMesh)
{
    if (MeshMap.find(InKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InKey].MeshLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }
    OutMesh = &(MeshMap[InKey].Meshes);
    return true;
}

bool MeshLoadHelper::GetBoundingMesh(const string& inPath, const string& inName, 
    DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
    shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh)
{
    string key = inPath + inName;
    return GetBoundingMesh(key, outSphere, outBox, outSphereMesh, outBoxMesh);
}
bool MeshLoadHelper::GetBoundingMesh(const string& InMeshKey,
    DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
    shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh)
{
    if (MeshMap.find(InMeshKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InMeshKey].MeshLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }

    outBox = MeshMap[InMeshKey].boundingBox;
    outBoxMesh = MeshMap[InMeshKey].boundingBoxMesh;
    outSphere = MeshMap[InMeshKey].boundingSphere;
    outSphereMesh = MeshMap[InMeshKey].boundingSphereMesh;

    return true;
}
string MeshLoadHelper::LoadBoxMesh(float InHalfExtent, bool bIndicesReverse)
{
    string Key = "Box"  + std::to_string(InHalfExtent);
    if (MeshMap.find(Key) == MeshMap.end())
    {
        std::vector<MeshData>& meshDatas = MeshMap[Key].MeshDatas;
        meshDatas = { GeometryGenerator::MakeBox(InHalfExtent) };
        if (bIndicesReverse)
        {
            std::reverse(meshDatas[0].indices.begin(), meshDatas[0].indices.end());
        }
        MeshMap[Key].MeshDataLoadType = hlab::ELoadType::Loaded;

        auto func = [Key]() {
            return LoadModel(Key); };
        hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
       tPool.EnqueueJob(func);
    }
    return Key;
}
}