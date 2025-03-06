#pragma once
#include <map>
#include <string>
#include <future>
#include <mutex>
#include "DModel2.h"
#include <directxtk/SimpleMath.h>
#include "GameDef.h"

namespace dengine {
using namespace std;
struct MeshBlock
{
	string PathName;
	string FileName;
	vector<DMesh> Meshes;
	vector<dengine::MeshData> MeshDatas;
	// Actor�� ������ ����
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

	std::future<vector<dengine::MeshData>> Loader;
	
	hlab::ELoadType MeshDataLoadType = hlab::ELoadType::NotLoaded;
	hlab::ELoadType MeshLoadType = hlab::ELoadType::NotLoaded;

};
class MeshLoadHelper
{
public:
	static void LoadAllGpuUnloadedModel();
	static bool GetMesh( const string& inPath, const string& inName, vector<DMesh>*& OutMesh);
	static bool GetMesh(const string& InKey, vector<DMesh>*& OutMesh);
	static bool GetBoundingMesh(const string& inPath, const string& inName, 
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh);
	static bool GetBoundingMesh(const string& InMeshKey,
		DirectX::BoundingSphere& outSphere, DirectX::BoundingBox& outBox,
		shared_ptr<DMesh>& outSphereMesh, shared_ptr<DMesh>& outBoxMesh);
	static bool LoadModel(const string& inPath, const string& inName);
	
	static void LoadModel(const string& InKey, vector<dengine::MeshData> MeshDatas);
	static bool GetMaterial(const string& inPath, const string& inName, MaterialConstants& InConstants);
	static bool GetMaterial(const string& InMeshKey, MaterialConstants& InConstants);
	static string LoadBoxMesh(float InHalfExtent, bool bIndicesReverse = false);
	static string LoadSquareMesh(const float scale = 1.0f,
		const Vector2 texScale = Vector2(1.0f));
private:
	static bool LoadModelCpuData(const string& inPath, const string& inName);
	static void LoadModelGpuData(const string& key);
public:
	static map<string, MeshBlock> s_meshMap;
	static std::mutex s_meshMapGuard;
};

}