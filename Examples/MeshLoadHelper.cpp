#include "MeshLoadHelper.h"
#include "Actor.h"
#include <thread>
namespace hlab {
vector<MeshData> MeshLoadHelper::LoadMeshData(Actor* InActor, const string& InPath, const string& InName)
{
	string key = InPath + InName;
	if (MeshMap.find(key) == MeshMap.end())
	{

	}
}
}