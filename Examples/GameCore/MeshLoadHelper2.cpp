#include "MeshLoadHelper2.h"
#include "../D3D12Core/Engine.h"
#include "../D3D12Core/Device.h"
#include "../D3D12Core/D3D12Utils.h"
#include "GeometryGenerator2.h"
#include "ThreadPool.h"
#include <filesystem>
#include "Mesh2.h"
#include <locale>
#include <codecvt>
namespace dengine {
using namespace DirectX;
map<string, MeshBlock> MeshLoadHelper::s_meshMap;
std::mutex MeshLoadHelper::s_meshMapGuard;
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(str);
}
BoundingBox GetBoundingBoxFromVertices(const vector<dengine::Vertex>& vertices) {

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

void MeshLoadHelper::LoadAllGpuUnloadedModel()
{
    for (auto& Pair : s_meshMap)
    {
        MeshBlock& mBloock = Pair.second;
        if (mBloock.MeshDataLoadType == hlab::ELoadType::Loading && mBloock.Loader._Is_ready() == true)
        {
            hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
            auto func = [&Pair]() {
                return LoadModelGpuData(Pair.first); };
            tPool.EnqueueJob(func);
        } 
    }
}

bool MeshLoadHelper::LoadModel(const string& inPath, const string& inName)
{
    return LoadModelCpuData(inPath, inName);
}
bool MeshLoadHelper::GetMaterial(const string& inPath, const string& inName, MaterialConstants& InConstants)
{
    string key = inPath + inName;
    return GetMaterial(key, InConstants);
}

bool MeshLoadHelper::GetMaterial(const string& InMeshKey, MaterialConstants& InConstants)
{
    if (s_meshMap.find(InMeshKey) == s_meshMap.end())
    {
        return false;
    }
    if (s_meshMap[InMeshKey].MeshDataLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }
    InConstants.useAlbedoMap = s_meshMap[InMeshKey].useAlbedoMap;
    InConstants.useAOMap = s_meshMap[InMeshKey].useAOMap;
    InConstants.useEmissiveMap = s_meshMap[InMeshKey].useEmissiveMap;
    InConstants.useMetallicMap = s_meshMap[InMeshKey].useMetalicMap;
    InConstants.useNormalMap = s_meshMap[InMeshKey].useNormalMap;
    InConstants.useRoughnessMap = s_meshMap[InMeshKey].useRoughnessMap;
    return true;
}

void MeshLoadHelper::LoadModel(const string& InKey, vector<dengine::MeshData> MeshDatas)
{
    if (s_meshMap.find(InKey) != s_meshMap.end())
    {
        return;
    }
    std::vector<MeshData>& meshDatas = s_meshMap[InKey].MeshDatas;
    meshDatas = MeshDatas;
    s_meshMap[InKey].MeshDataLoadType = hlab::ELoadType::Loaded;
    LoadModelGpuData(InKey);
}

bool MeshLoadHelper::GetMesh(const string& inPath, const string& inName, vector<DMesh>*& OutMesh)
{
    string key = inPath + inName;
    return GetMesh(key, OutMesh);
}

bool MeshLoadHelper::GetMesh(const string& InKey, vector<DMesh>*& OutMesh)
{
    if (s_meshMap.find(InKey) == s_meshMap.end())
    {
        return false;
    }
    if (s_meshMap[InKey].MeshLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }
    OutMesh = &(s_meshMap[InKey].Meshes);
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
    if (s_meshMap.find(InMeshKey) == s_meshMap.end())
    {
        return false;
    }
    if (s_meshMap[InMeshKey].MeshLoadType != hlab::ELoadType::Loaded)
    {
        return false;
    }

    outBox = s_meshMap[InMeshKey].boundingBox;
    outBoxMesh = s_meshMap[InMeshKey].boundingBoxMesh;
    outSphere = s_meshMap[InMeshKey].boundingSphere;
    outSphereMesh = s_meshMap[InMeshKey].boundingSphereMesh;

    return true;
}
string MeshLoadHelper::LoadBoxMesh(float InHalfExtent, bool bIndicesReverse)
{
    string Key = "Box"  + std::to_string(InHalfExtent);
    if (s_meshMap.find(Key) == s_meshMap.end())
    {
        std::vector<MeshData>& meshDatas = s_meshMap[Key].MeshDatas;
        meshDatas = { GeometryGenerator::MakeBox(InHalfExtent) };
        if (bIndicesReverse)
        {
            std::reverse(meshDatas[0].indices.begin(), meshDatas[0].indices.end());
        }
        s_meshMap[Key].MeshDataLoadType = hlab::ELoadType::Loaded;

        auto func = [Key]() {
            return LoadModelGpuData(Key); };
        hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
       tPool.EnqueueJob(func);
    }
    return Key;
}

string MeshLoadHelper::LoadSquareMesh(const float scale, const Vector2 texScale)
{
    // TODO. texScale도 키로
    string Key = "Square" + std::to_string(scale);
    if (s_meshMap.find(Key) == s_meshMap.end())
    {
        std::vector<MeshData>& meshDatas = s_meshMap[Key].MeshDatas;
        meshDatas = { GeometryGenerator::MakeSquare(scale, texScale) };
        s_meshMap[Key].MeshDataLoadType = hlab::ELoadType::Loaded;

        auto func = [Key]() {
            return LoadModelGpuData(Key); };
        hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
        tPool.EnqueueJob(func);
    }
    return Key;
}

bool MeshLoadHelper::LoadModelCpuData(const string& inPath, const string& inName)
{
    // NOTICE. 메인 스레드에서만 실행중이라 아직은 동시 접근이 안됨
    string key = inPath + inName;
    if (s_meshMap.find(key) == s_meshMap.end())
    {
        s_meshMap[key] = MeshBlock();

        MeshBlock& meshBlocks = s_meshMap[key];
        meshBlocks.PathName = inPath;
        meshBlocks.FileName = inName;

        hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
        auto func = [&meshBlocks]() {
            return CreateMeshData(meshBlocks); };
        s_meshMap[key].Loader = tPool.EnqueueJob(func);
        s_meshMap[key].MeshDataLoadType = hlab::ELoadType::Loading;
        return false;
    }
    return s_meshMap[key].MeshDataLoadType == hlab::ELoadType::Loaded;
}


void MeshLoadHelper::LoadModelGpuData(const string& key)
{
    {
        std::lock_guard<std::mutex> lock(MeshLoadHelper::s_meshMapGuard);

        // cpu 데이터 로드 안됨
        if (s_meshMap.find(key) == s_meshMap.end()
            || s_meshMap[key].MeshDataLoadType == hlab::ELoadType::NotLoaded)
        {
            return;
        }

        // cpu 데이터 로딩중
        if (s_meshMap[key].MeshDataLoadType == hlab::ELoadType::Loading && s_meshMap[key].Loader._Is_ready() == false)
        {
            return;
        }
        // 이미 gpu 데이터 로딩중 or Loaded
        if (s_meshMap[key].MeshLoadType == hlab::ELoadType::Loading
            || s_meshMap[key].MeshLoadType == hlab::ELoadType::Loaded)
        {
            return;
        }
        s_meshMap[key].MeshLoadType = hlab::ELoadType::Loading;
    }

    std::vector<MeshData>& meshDatas = s_meshMap[key].MeshDatas;
    if (s_meshMap[key].MeshDataLoadType == hlab::ELoadType::Loading && s_meshMap[key].Loader._Is_ready() == true)
    {
        meshDatas = s_meshMap[key].Loader.get();
        s_meshMap[key].MeshDataLoadType = hlab::ELoadType::Loaded;
    }

    MeshBlock& meshBlock = s_meshMap[key];
    vector<DMesh>& meshes = meshBlock.Meshes;
    int index = 0;
    for (const auto& meshData : meshDatas) {
        if (meshes.size() <= index)
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
                    D3D12Utils::LoadAlbedoOpacityTexture(
                        DEVICE, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        tex2D);
                    tex2D->SetName(L"albedoTexture");
                    newMesh.albedoTexture = std::make_shared<Texture>();
                    newMesh.albedoTexture->CreateFromResource(tex2D);
                    newMesh.albedoTexture->SetRegister(SRV_REGISTER::t1);
                }
                else
                {
                    newMesh.albedoTexture = std::make_shared<Texture>();
                    wstring path = string_to_wstring(meshData.albedoTextureFilename);
                    newMesh.albedoTexture->LoadTexture(path, false, true, false);
                    newMesh.albedoTexture->SetRegister(SRV_REGISTER::t1);
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
                newMesh.emissiveTexture = std::make_shared<Texture>();
                wstring path = string_to_wstring(meshData.emissiveTextureFilename);
                newMesh.emissiveTexture->LoadTexture(path, false, true, false);
                newMesh.emissiveTexture->SetRegister(SRV_REGISTER::t5);
                meshBlock.useEmissiveMap = true;
            }
            else
            {
                cout << meshData.emissiveTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename))
            {
                newMesh.normalTexture = std::make_shared<Texture>();
                wstring path = string_to_wstring(meshData.normalTextureFilename);
                newMesh.normalTexture->LoadTexture(path, false, true, false);
                newMesh.normalTexture->SetRegister(SRV_REGISTER::t2);
                meshBlock.useNormalMap = true;
            }
            else {
                cout << meshData.normalTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename))
            {
                newMesh.heightTexture = std::make_shared<Texture>();
                wstring path = string_to_wstring(meshData.heightTextureFilename);
                newMesh.heightTexture->LoadTexture(path, false, true, false);
                newMesh.heightTexture->SetRegister(SRV_REGISTER::t0);
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
                newMesh.aoTexture = std::make_shared<Texture>();
                wstring path = string_to_wstring(meshData.aoTextureFilename);
                newMesh.aoTexture->LoadTexture(path, false, false, false);
                newMesh.aoTexture->SetRegister(SRV_REGISTER::t3);
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
                if (meshData.metallicTextureFilename == meshData.roughnessTextureFilename)
                {
                    newMesh.metallicRoughnessTexture = std::make_shared<Texture>();
                    wstring path = string_to_wstring(meshData.metallicTextureFilename);
                    newMesh.metallicRoughnessTexture->LoadTexture(path, false, false, false);
                    newMesh.metallicRoughnessTexture->SetRegister(SRV_REGISTER::t4);
                }
                else
                {
                    ComPtr<ID3D12Resource> tex2D;
                    D3D12Utils::LoadMetallicRoughnessTexture(
                        DEVICE, meshData.metallicTextureFilename,
                        meshData.roughnessTextureFilename, tex2D);
                    tex2D->SetName(L"metallicRoughnessTexture");
                    newMesh.metallicRoughnessTexture = std::make_shared<Texture>();
                    newMesh.metallicRoughnessTexture->CreateFromResource(tex2D);
                    newMesh.metallicRoughnessTexture->SetRegister(SRV_REGISTER::t4);
                }
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
    s_meshMap[key].MeshLoadType = hlab::ELoadType::Loaded;
}

}


