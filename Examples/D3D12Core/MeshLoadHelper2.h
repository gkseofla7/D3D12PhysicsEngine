#pragma once
#include <map>
#include <string>
#include <future>
#include <mutex>
#include "DModel2.h"
#include <directxtk/SimpleMath.h>
#include "../GameDef.h"

namespace hlab {
using namespace std;
struct MeshBlock
{
	string PathName;
	string FileName;
	vector<DMesh> Meshes;
	vector<hlab::MeshData> MeshDatas;
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
	shared_ptr<DMesh> boundingBoxMesh;
	shared_ptr<DMesh> boundingSphereMesh;

	std::future<vector<hlab::MeshData>> Loader;
	
	hlab::ELoadType MeshDataLoadType = hlab::ELoadType::NotLoaded;
	hlab::ELoadType MeshLoadType = hlab::ELoadType::NotLoaded;

};
class MeshLoadHelper2
{
public:
	static void LoadAllUnloadedModel();
	static bool GetMesh( const string& inPath, const string& inName, vector<DMesh>*& OutMesh);
	static bool GetMesh(const string& InKey, vector<DMesh>*& OutMesh);
	static bool GetBoundingMesh(const string& inPath, const string& inName, 
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh);
	static bool GetBoundingMesh(const string& InMeshKey,
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh);

	static bool LoadModelData(const string& inPath, const string& inName);
	static void LoadModel(const string& key);
	static bool GetMaterial(const string& inPath, const string& inName, MaterialConstants2& InConstants);
	static bool GetMaterial(const string& InMeshKey, MaterialConstants2& InConstants);
	static string LoadBoxMesh(float InHalfExtent);
public:
	static map<string, MeshBlock> MeshMap;
	static std::mutex m_mtx;
};

}