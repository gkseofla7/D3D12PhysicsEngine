#include "MeshLoadHelper.h"
#include "Actor.h"

#include "GeometryGenerator.h"
#include "ConstantBuffers.h"
#include "ThreadPool.h"
#include <filesystem>
#include <mutex>
namespace hlab {
    using namespace DirectX;
    map<string, MeshBlock> MeshLoadHelper::MeshMap;
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

void MeshLoadHelper::LoadUnloadedModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
    for (auto& Pair : MeshMap)
    {
        MeshBlock& mBloock = Pair.second;
        if (mBloock.IsLoading == true && mBloock.Loader._Is_ready() == true)
        {
            ThreadPool& tPool = ThreadPool::getInstance();
            //음.. 이 순간 저 값들을 캡쳐하는게..ㅋㅋ
            auto func = [&device,&context, &Pair]() {
                return LoadModel(device, nullptr, Pair.first); };
            mBloock.LoadCommandList = tPool.EnqueueJob(func);
        } 
        else if (mBloock.IsModelLoading == true && mBloock.LoadCommandList._Is_ready() == true)
        { 
            // 4. 주 스레드에서 Immediate Context를 사용해 명령 리스트 실행
            ID3D11CommandList* CommandList = mBloock.LoadCommandList.get();
            context->ExecuteCommandList(CommandList, FALSE);
            CommandList->Release();
            mBloock.deferredContext = nullptr; 
            //mBloock.deferredContext->Release();
            LoadModel(device, context, Pair.first); 
            mBloock.IsModelLoading = false;
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
		MeshMap[key].IsLoading = true;
		return false;
	}
	
    return true;
}
bool MeshLoadHelper::SetMaterial(const string& InPath, const string& InName, MaterialConstants& InConstants)
{
    string key = InPath + InName;
    if (MeshMap.find(key) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[key].IsLoading == true)
    {
        return false;
    }
    InConstants.useAlbedoMap = MeshMap[key].useAlbedoMap;
    InConstants.useAOMap = MeshMap[key].useAOMap;
    InConstants.useEmissiveMap = MeshMap[key].useEmissiveMap;
    InConstants.useMetallicMap = MeshMap[key].useMetalicMap;
    InConstants.useNormalMap = MeshMap[key].useNormalMap;
    InConstants.useRoughnessMap = MeshMap[key].useRoughnessMap;
    return true;
}
ID3D11CommandList* MeshLoadHelper::LoadModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const string& key)
{
    bool deferred = context == nullptr;
    if (deferred)
    {
        if (MeshMap.find(key) == MeshMap.end())
        {
            return nullptr;
        }
        // MeshData 로드 안됨
        if (MeshMap[key].IsLoading == false)
        {
            return nullptr;
        }
        // MeshData 로딩중
        if (MeshMap[key].IsLoading == true && MeshMap[key].Loader._Is_ready() == false)
        {
            return nullptr;
        }
        // 이미 모델 로딩중
        if (MeshMap[key].IsModelLoading)
        {
            return nullptr;
        }
    }
    
    // 1. Deferred Context 생성
    std::vector<MeshData>& MeshDatas = MeshMap[key].MeshDatas;
    MeshMap[key].IsLoading = false;
    
    ComPtr<ID3D11DeviceContext> deferredContext = nullptr;
    if (deferred)
    {
        MeshMap[key].IsModelLoading = true;
        device->CreateDeferredContext(0, &deferredContext); 
        MeshMap[key].deferredContext = deferredContext;
        MeshDatas = MeshMap[key].Loader.get();
    }
    else
    {
        deferredContext = context; 
    }
    
    
    // 이 부분도 따로 멀티스레딩으로 뺄수없을까?
    MeshBlock& MeshBlock = MeshMap[key];
    vector<Mesh>& meshes = MeshBlock.Meshes;
    int index = 0;
    for (const auto& meshData : MeshDatas) { 
        if (meshes.size()<= index)
        {
            meshes.push_back(Mesh());
            
        }
        Mesh& newMesh = meshes[index];
         
        //TODO. Skinned일때만, 아닌경우 조건문 추가 예정
        if (deferred)
        {
            D3D11Utils::CreateVertexBuffer(device, meshData.skinnedVertices,
                newMesh.vertexBuffer);
            newMesh.indexCount = UINT(meshData.indices.size());
            newMesh.vertexCount = UINT(meshData.skinnedVertices.size());
            newMesh.stride = UINT(sizeof(SkinnedVertex));
            D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                newMesh.indexBuffer);
        }


        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    D3D11Utils::CreateTexture(
                        device, deferredContext, meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        newMesh.albedoTexture, newMesh.albedoSRV, deferred);
                }
                else {
                    D3D11Utils::CreateTexture(
                        device, deferredContext, meshData.albedoTextureFilename, true,
                        newMesh.albedoTexture, newMesh.albedoSRV, deferred);
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
                    device, deferredContext, meshData.emissiveTextureFilename, true,
                    newMesh.emissiveTexture, newMesh.emissiveSRV, deferred);
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
                    device, deferredContext, meshData.normalTextureFilename, false,
                    newMesh.normalTexture, newMesh.normalSRV, deferred);
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
                    device, deferredContext, meshData.heightTextureFilename, false,
                    newMesh.heightTexture, newMesh.heightSRV, deferred);
                MeshBlock.useHeightMap = true;
            }
            else {
                cout << meshData.heightTextureFilename
                    << " does not exists. Skip texture reading." << endl;
            }
        }
         
        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                D3D11Utils::CreateTexture(device, deferredContext,
                    meshData.aoTextureFilename, false,
                    newMesh.aoTexture, newMesh.aoSRV, deferred);
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
                    device, deferredContext, meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    newMesh.metallicRoughnessTexture,
                    newMesh.metallicRoughnessSRV, deferred);
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
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            MeshBlock.boundingBoxMesh->vertexBuffer);
        MeshBlock.boundingBoxMesh->indexCount = UINT(meshData.indices.size());
        MeshBlock.boundingBoxMesh->vertexCount = UINT(meshData.vertices.size());
        MeshBlock.boundingBoxMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
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
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
            MeshBlock.boundingSphereMesh->vertexBuffer);
        MeshBlock.boundingSphereMesh->indexCount = UINT(meshData.indices.size());
        MeshBlock.boundingSphereMesh->vertexCount = UINT(meshData.vertices.size());
        MeshBlock.boundingSphereMesh->stride = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
            MeshBlock.boundingSphereMesh->indexBuffer);
    }

    // 3. 명령 리스트로 마무리
    if (deferred)
    {
        ID3D11CommandList* commandList = nullptr;
        deferredContext->FinishCommandList(FALSE, &commandList);
        // 4. 주 스레드에서 Immediate Context를 사용해 명령 리스트 실행
        return commandList;
    }
    else
    {
        return nullptr;
    }
}

bool MeshLoadHelper::GetMesh(const string& InPath, const string& InName, vector<Mesh>*& OutMesh)
{
    string key = InPath + InName;
    if (MeshMap.find(key) == MeshMap.end())
    {
        return false;
    }
    if (MeshMap[key].IsLoading == true || MeshMap[key].IsModelLoading == true)
    {
        return false;
    }
    OutMesh = &(MeshMap[key].Meshes);
    return true;
}
}