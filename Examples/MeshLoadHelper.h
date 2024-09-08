#pragma once
#include <map>
#include <string>
#include <future>
#include "dModel.h"
#include <directxtk/SimpleMath.h>

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
	std::future<ID3D11CommandList*> LoadCommandList;
	bool IsLoading = false;
	bool IsModelLoading = false;
};
class MeshLoadHelper
{
public:
	static void LoadUnloadedModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
	static bool GetMesh( const string& InPath, const string& InName, vector<Mesh>*& OutMesh);
	static bool LoadModelData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,const string& InPath, const string& InName, vector<Mesh>* OutModel);
	static ID3D11CommandList* LoadModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context,const string& key);
	static bool SetMaterial(const string& InPath, const string& InName, MaterialConstants& InConstants);
public:
	static map<string, MeshBlock> MeshMap;
};

}