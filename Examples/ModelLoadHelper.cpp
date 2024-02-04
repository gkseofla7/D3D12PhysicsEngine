#include "ModelLoadHelper.h"
#include "Actor.h"
#include "ThreadPool.h"
namespace hlab {
DModel CreateModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, string path, string name)
{
	return DModel(device,context,path,name);
}

bool ModelLoadHelper::LoadModelData(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const string& InPath, const string& InName, DModel* OutMeshData)
{
	string key = InPath + InName;
	if (ModelMap.find(key) == ModelMap.end())
	{
		ModelMap[key] = ModelBlock();

		ThreadPool& tPool =  ThreadPool::getInstance();
		ModelMap[key].Loader = tPool.EnqueueJob(CreateModel, device, context,InPath, InName);
		ModelMap[key].IsLoading = true;
		return false;
	}
	if (ModelMap[key].IsLoading ==true && ModelMap[key].Loader._Is_ready() == false)
	{
		return false;
	}
	ModelMap[key].IsLoading = false;
	ModelMap[key].Model = ModelMap[key].Loader.get();
	OutMeshData = &(ModelMap[key].Model);
	return true;
}

bool ModelLoadHelper::GetModelData(const string& InPath, const string& InName, DModel* OutModel)
{
	string key = InPath + InName;
	if (ModelMap.find(key) == ModelMap.end())
	{
		return false;
	}
	if (ModelMap[key].IsLoading == true && ModelMap[key].Loader._Is_ready() == false)
	{
		return false;
	}
	ModelMap[key].IsLoading = false;
	ModelMap[key].Model = ModelMap[key].Loader.get();
	OutModel = &(ModelMap[key].Model);
	return true;
}
}