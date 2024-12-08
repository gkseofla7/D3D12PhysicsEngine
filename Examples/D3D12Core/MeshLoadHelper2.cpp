#include "MeshLoadHelper2.h"
#include "Engine.h"
#include "Device.h"
#include "D3D12Utils.h"
#include "../GeometryGenerator.h"
#include "../ThreadPool.h"
#include <filesystem>

namespace hlab {
    using namespace DirectX;
    map<string, MeshBlock> MeshLoadHelper2::MeshMap;
    std::mutex MeshLoadHelper2::m_mtx;
    BoundingBox GetBoundingBoxFromVertices(const vector<hlab::Vertex>& vertices) {

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

    void GetExtendBoundingBox(const BoundingBox& inBox, BoundingBox& outBox) {

        Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
        Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

        minCorner = Vector3::Min(minCorner,
            Vector3(outBox.Center) - Vector3(outBox.Extents));
        maxCorner = Vector3::Max(maxCorner,
            Vector3(outBox.Center) + Vector3(outBox.Extents));

        outBox.Center = (minCorner + maxCorner) * 0.5f;
        outBox.Extents = maxCorner - outBox.Center;
    }

vector<MeshData> CreateMeshData(MeshBlock& OutMeshBlock)
{ 
	auto [mesheDatas, _] =
		GeometryGenerator::ReadAnimationFromFile(OutMeshBlock.PathName, OutMeshBlock.FileName);
    return mesheDatas;
}
AnimationData ReadAnimationFromFile(string path, string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}

void MeshLoadHelper2::LoadAllUnloadedModel()
{
    for (auto& Pair : MeshMap)
    {
        MeshBlock& mBloock = Pair.second;
        if (mBloock.MeshDataLoadType == ELoadType::Loading && mBloock.Loader._Is_ready() == true)
        {
            ThreadPool& tPool = ThreadPool::getInstance();
            //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
            auto func = [&Pair]() {
                return LoadModel(Pair.first); };
            tPool.EnqueueJob(func);
        } 
    }
}
bool MeshLoadHelper2::LoadModelData( const string& inPath, const string& inName)
{
	string key = inPath + inName;
	if (MeshMap.find(key) == MeshMap.end())
	{
		MeshMap[key] = MeshBlock();
         
        MeshBlock& meshBlocks = MeshMap[key];
        meshBlocks.PathName = inPath;
        meshBlocks.FileName = inName;

		ThreadPool& tPool =  ThreadPool::getInstance();
        //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
        auto func = [&meshBlocks]() {
            return CreateMeshData(meshBlocks); };
		MeshMap[key].Loader = tPool.EnqueueJob(func);
		MeshMap[key].MeshDataLoadType = ELoadType::Loading;
		return false;
	}
	
    return MeshMap[key].MeshDataLoadType == ELoadType::Loaded;
}
bool MeshLoadHelper2::GetMaterial(const string& inPath, const string& inName, MaterialConstants& InConstants)
{
    string key = inPath + inName;
    return GetMaterial(key, inName, InConstants);
}
bool MeshLoadHelper2::GetMaterial(const string& InMeshKey, MaterialConstants& InConstants)
{
    if (MeshMap.find(InMeshKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InMeshKey].MeshDataLoadType != ELoadType::Loaded)
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
void MeshLoadHelper2::LoadModel(const string& key)
{
    {
        std::lock_guard<std::mutex> lock(MeshLoadHelper2::m_mtx);

        if (MeshMap.find(key) == MeshMap.end())
        {
            return;
        }
        // MeshData 로드 안됨
        if (MeshMap[key].MeshDataLoadType == ELoadType::NotLoaded)
        {
            return;
        }
        // MeshData 로딩중
        if (MeshMap[key].MeshDataLoadType == ELoadType::Loading && MeshMap[key].Loader._Is_ready() == false)
        {
            return;
        }
        // 이미 모델 로딩중 or Loaded
        if (MeshMap[key].MeshLoadType == ELoadType::Loading
            || MeshMap[key].MeshLoadType == ELoadType::Loaded)
        {
            return;
        }
        MeshMap[key].MeshLoadType = ELoadType::Loading;
    }

    std::vector<MeshData>& meshDatas = MeshMap[key].MeshDatas;
    if (MeshMap[key].MeshDataLoadType == ELoadType::Loading && MeshMap[key].Loader._Is_ready() == true)
    {
        meshDatas = MeshMap[key].Loader.get();
        MeshMap[key].MeshDataLoadType = ELoadType::Loaded;
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
                    newMesh.albedoTexture.CreateFromResource(tex2D);
                }
                else 
                {
                    ComPtr<ID3D12Resource> tex2D;
                    D3D12Utils::CreateTexture(
                        DEVICE, meshData.albedoTextureFilename, true,
                        tex2D);
                    newMesh.albedoTexture.CreateFromResource(tex2D);
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
                newMesh.emissiveTexture.CreateFromResource(tex2D);
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
                newMesh.normalTexture.CreateFromResource(tex2D);
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
                newMesh.heightTexture.CreateFromResource(tex2D);
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
                newMesh.aoTexture.CreateFromResource(tex2D);
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
                newMesh.metallicRoughnessTexture.CreateFromResource(tex2D);
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
        meshBlock.boundingBox = GetBoundingBoxFromVertices(meshDatas[0].vertices);
        for (size_t i = 1; i < meshDatas.size(); i++) {
            auto bb = GetBoundingBoxFromVertices(meshDatas[0].vertices);
            GetExtendBoundingBox(bb, meshBlock.boundingBox);
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
    MeshMap[key].MeshLoadType = ELoadType::Loaded;
}


bool MeshLoadHelper2::GetMesh(const string& inPath, const string& inName, vector<DMesh>*& OutMesh)
{
    string key = inPath + inName;
    return GetMesh(key, OutMesh);
}

bool MeshLoadHelper2::GetMesh(const string& InKey, vector<DMesh>*& OutMesh)
{
    if (MeshMap.find(InKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InKey].MeshLoadType != ELoadType::Loaded)
    {
        return false;
    }
    OutMesh = &(MeshMap[InKey].Meshes);
    return true;
}

bool MeshLoadHelper2::GetBoundingMesh(const string& inPath, const string& inName, 
    DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
    shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh)
{
    string key = inPath + inName;
    return GetBoundingMesh(key, outSphere, outBox, outSphereMesh, outBoxMesh);
}
bool MeshLoadHelper2::GetBoundingMesh(const string& InMeshKey,
    DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
    shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh)
{
    if (MeshMap.find(InMeshKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InMeshKey].MeshLoadType != ELoadType::Loaded)
    {
        return false;
    }

    outBox = MeshMap[InMeshKey].boundingBox;
    outBoxMesh = MeshMap[InMeshKey].boundingBoxMesh;
    outSphere = MeshMap[InMeshKey].boundingSphere;
    outSphereMesh = MeshMap[InMeshKey].boundingSphereMesh;

    return true;
}
string MeshLoadHelper2::LoadBoxMesh(float InHalfExtent)
{
    string Key = "Box"  + std::to_string(InHalfExtent);
    if (MeshMap.find(Key) == MeshMap.end())
    {
        std::vector<MeshData>& meshDatas = MeshMap[Key].MeshDatas;
        meshDatas = { GeometryGenerator::MakeBox(InHalfExtent) };
        MeshMap[Key].MeshDataLoadType = ELoadType::Loaded;

        auto func = [Key]() {
            return LoadModel(Key); };
        ThreadPool& tPool = ThreadPool::getInstance();
       tPool.EnqueueJob(func);
    }
    return Key;
}
}