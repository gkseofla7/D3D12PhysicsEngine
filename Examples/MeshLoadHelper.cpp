#include "MeshLoadHelper.h"
#include "Actor.h"

#include "GeometryGenerator.h"
#include "ConstantBuffers.h"
#include "ThreadPool.h"
#include <filesystem>

namespace hlab {
    using namespace DirectX;
    map<string, MeshBlock> MeshLoadHelper::MeshMap;
    std::mutex MeshLoadHelper::m_mtx;
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

void MeshLoadHelper::LoadAllUnloadedModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
    for (auto& Pair : MeshMap)
    {
        MeshBlock& mBloock = Pair.second;
        if (mBloock.MeshDataLoadType == ELoadType::Loading && mBloock.Loader._Is_ready() == true)
        {
            ThreadPool& tPool = ThreadPool::getInstance();
            //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
            auto func = [&device,&context, &Pair]() {
                return LoadModel(device, context, Pair.first); };
            tPool.EnqueueJob(func);
        } 
    }
}
bool MeshLoadHelper::LoadModelData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const string& InPath, const string& InName, vector<Mesh>* OutMeshes)
{
	string key = InPath + InName;
	if (MeshMap.find(key) == MeshMap.end())
	{
		MeshMap[key] = MeshBlock();
         
        MeshBlock& meshBlocks = MeshMap[key];
        meshBlocks.PathName = InPath;
        meshBlocks.FileName = InName;

		ThreadPool& tPool =  ThreadPool::getInstance();
        //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
        auto func = [&device, &context, &meshBlocks]() {
            return CreateMeshData(meshBlocks); };
		MeshMap[key].Loader = tPool.EnqueueJob(func);
		MeshMap[key].MeshDataLoadType = ELoadType::Loading;
		return false;
	}
	
    return MeshMap[key].MeshDataLoadType == ELoadType::Loaded;
}
bool MeshLoadHelper::GetMaterial(const string& InPath, const string& InName, MaterialConstants& InConstants)
{
    string key = InPath + InName;
    return GetMaterial(key, InName, InConstants);
}
bool MeshLoadHelper::GetMaterial(const string& InMeshKey, MaterialConstants& InConstants)
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
void MeshLoadHelper::LoadModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const string& key)
{
    {
        std::lock_guard<std::mutex> lock(MeshLoadHelper::m_mtx);

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

        // 동시에 여기에 들어오면 이슈 발생,,
        MeshMap[key].MeshLoadType = ELoadType::Loading;
    }

    std::vector<MeshData>& MeshDatas = MeshMap[key].MeshDatas;
    if (MeshMap[key].MeshDataLoadType == ELoadType::Loading && MeshMap[key].Loader._Is_ready() == true)
    {
        MeshDatas = MeshMap[key].Loader.get();
        MeshMap[key].MeshDataLoadType = ELoadType::Loaded;
    }

    MeshBlock& MeshBlock = MeshMap[key];
    vector<Mesh>& meshes = MeshBlock.Meshes;
    int index = 0;
    for (const auto& meshData : MeshDatas) { 
        if (meshes.size()<= index)
        {
            meshes.push_back(Mesh());
            
        }
        Mesh& newMesh = meshes[index];
         
        if (meshData.skinnedVertices.size() > 0)
        {
            D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                newMesh.vertexBuffer);
            newMesh.indexCount = UINT(meshData.indices.size());
            newMesh.vertexCount = UINT(meshData.skinnedVertices.size());
            newMesh.stride = UINT(sizeof(SkinnedVertex));
            D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                newMesh.indexBuffer);
        }
        else
        {
            D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                newMesh.vertexBuffer);
            newMesh.indexCount = UINT(meshData.indices.size());
            newMesh.vertexCount = UINT(meshData.vertices.size());
            newMesh.stride = UINT(sizeof(Vertex));
            D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                newMesh.indexBuffer);
        }

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
                MeshBlock.useAlbedoMap = true;
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
                MeshBlock.useEmissiveMap = true;
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
                MeshBlock.useNormalMap = true;
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
                MeshBlock.useHeightMap = true;
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
                MeshBlock.useAOMap = true;
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
            MeshBlock.useMetalicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            MeshBlock.useRoughnessMap = true;
        }
        index++;
    } 
    // Initialize Bounding Box
    {
        MeshBlock.boundingBox = GetBoundingBoxFromVertices(MeshDatas[0].vertices);
        for (size_t i = 1; i < MeshDatas.size(); i++) {
            auto bb = GetBoundingBoxFromVertices(MeshDatas[0].vertices);
            GetExtendBoundingBox(bb, MeshBlock.boundingBox);
        }

        auto meshData = GeometryGenerator::MakeWireBox(
            MeshBlock.boundingBox.Center,
            Vector3(MeshBlock.boundingBox.Extents) + Vector3(1e-3f));
        MeshBlock.boundingBoxMesh = std::make_shared<Mesh>();

        MeshBlock.boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        MeshBlock.boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        MeshBlock.boundingBoxMesh->stride = UINT(sizeof(Vertex));

        D3D11Utils::CreateVertexBuffer(device, std::move(meshData.vertices),
            MeshBlock.boundingBoxMesh->vertexBuffer);
        D3D11Utils::CreateIndexBuffer(device, std::move(meshData.indices),
            MeshBlock.boundingBoxMesh->indexBuffer);
    }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto& mesh : MeshDatas) {
            for (auto& v : mesh.vertices) {
                maxRadius = std::max(
                    (Vector3(MeshBlock.boundingBox.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // 살짝 크게 설정
        MeshBlock.boundingSphere = BoundingSphere(MeshBlock.boundingBox.Center, maxRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            MeshBlock.boundingSphere.Center, MeshBlock.boundingSphere.Radius);
        MeshBlock.boundingSphereMesh = std::make_shared<Mesh>();

        MeshBlock.boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        MeshBlock.boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        MeshBlock.boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateVertexBuffer(device, std::move(meshData.vertices),
            MeshBlock.boundingSphereMesh->vertexBuffer);
        D3D11Utils::CreateIndexBuffer(device, std::move(meshData.indices),
            MeshBlock.boundingSphereMesh->indexBuffer);
    }
    MeshMap[key].MeshLoadType = ELoadType::Loaded;
}


bool MeshLoadHelper::GetMesh(const string& InPath, const string& InName, vector<Mesh>*& OutMesh)
{
    string key = InPath + InName;
    return GetMesh(key, OutMesh);
}

bool MeshLoadHelper::GetMesh(const string& InKey, vector<Mesh>*& OutMesh)
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

bool MeshLoadHelper::GetBoundingMesh(const string& InPath, const string& InName, 
    DirectX::BoundingSphere& OutSphere, DirectX::BoundingBox& OutBox,
    shared_ptr<Mesh>& OutSphereMesh, shared_ptr<Mesh>& OutBoxMesh)
{
    string key = InPath + InName;
    return GetBoundingMesh(key, OutSphere, OutBox, OutSphereMesh, OutBoxMesh);
}
bool MeshLoadHelper::GetBoundingMesh(const string& InMeshKey,
    DirectX::BoundingSphere& OutSphere, DirectX::BoundingBox& OutBox,
    shared_ptr<Mesh>& OutSphereMesh, shared_ptr<Mesh>& OutBoxMesh)
{
    if (MeshMap.find(InMeshKey) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[InMeshKey].MeshLoadType != ELoadType::Loaded)
    {
        return false;
    }

    OutBox = MeshMap[InMeshKey].boundingBox;
    OutBoxMesh = MeshMap[InMeshKey].boundingBoxMesh;
    OutSphere = MeshMap[InMeshKey].boundingSphere;
    OutSphereMesh = MeshMap[InMeshKey].boundingSphereMesh;

    return true;
}
string MeshLoadHelper::LoadBoxMesh(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float InHalfExtent)
{
    string Key = "Box"  + std::to_string(InHalfExtent);
    if (MeshMap.find(Key) == MeshMap.end())
    {
        std::vector<MeshData>& MeshDatas = MeshMap[Key].MeshDatas;
        MeshDatas = { GeometryGenerator::MakeBox(InHalfExtent) };
        MeshMap[Key].MeshDataLoadType = ELoadType::Loaded;

        auto func = [&device, &context, Key]() {
            return LoadModel(device, context, Key); };
        ThreadPool& tPool = ThreadPool::getInstance();
       tPool.EnqueueJob(func);
    }
    return Key;
}
}