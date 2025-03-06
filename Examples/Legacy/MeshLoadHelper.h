#pragma once
#include <map>
#include <string>
#include <future>
#include <mutex>
#include "dModel.h"
#include <directxtk/SimpleMath.h>
#include "GameDef.h"

namespace hlab {
using namespace std;
struct MeshBlock
{
	string PathName;
	string FileName;
	vector<Mesh> Meshes;
	vector<MeshData> MeshDatas;
	// Actor에 전달할 값들
	bool useAlbedoMap = false;
	bool useEmissiveMap = false;
	bool useNormalMap = false;
	bool useHeightMap = false;
	bool useAOMap = false;
	bool useMetalicMap = false;
	bool useRoughnessMap = false;
	// BoundingVolume
	DirectX::BoundingBox boundingBox;
	DirectX::BoundingSphere boundingSphere;
	shared_ptr<Mesh> boundingBoxMesh;
	shared_ptr<Mesh> boundingSphereMesh;

	ComPtr<ID3D11DeviceContext> deferredContext;
	std::future<vector<MeshData>> Loader;
	
	ELoadType MeshDataLoadType = ELoadType::NotLoaded;
	ELoadType MeshLoadType = ELoadType::NotLoaded;

};
class MeshLoadHelper
{
public:
	static void LoadAllGpuUnloadedModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
	static bool GetMesh( const string& inPath, const string& inName, vector<Mesh>*& OutMesh);
	static bool GetMesh(const string& InKey, vector<Mesh>*& OutMesh);
	static bool GetBoundingMesh(const string& inPath, const string& inName, 
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<Mesh>& outSphereMesh, shared_ptr<Mesh>& outBoxMesh);
	static bool GetBoundingMesh(const string& InMeshKey,
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<Mesh>& outSphereMesh, shared_ptr<Mesh>& outBoxMesh);

	static bool LoadModel(const string& inPath, const string& inName);
	static void LoadModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,const string& key);
	static bool GetMaterial(const string& inPath, const string& inName, MaterialConstants& InConstants);
	static bool GetMaterial(const string& InMeshKey, MaterialConstants& InConstants);
	static string LoadBoxMesh(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float InHalfExtent);
public:
	static map<string, MeshBlock> MeshMap;
	static std::mutex m_mtx;
};

}