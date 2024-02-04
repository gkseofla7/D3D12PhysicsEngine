#pragma once
#include <map>
#include <string>
#include <future>
#include <thread>
#include "MeshData.h"
#include "dModel.h"
namespace hlab {
using namespace std;
struct ModelBlock
{
	string PathName;
	DModel Model;
	std::future<DModel> Loader;
	bool IsLoading = false;
};
class ModelLoadHelper
{
public:
	static bool GetModelData(const string& InPath, const string& InName, DModel* OutModel);
	static bool LoadModelData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const string& InPath, const string& InName, DModel* OutModel);
public:
	static map<string, ModelBlock> ModelMap;
};

}