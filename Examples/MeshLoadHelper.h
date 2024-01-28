#pragma once
#include <map>
#include <string>
#include "MeshData.h"
namespace hlab {
using namespace std;
class MeshLoadHelper
{
	static vector<MeshData> LoadMeshData(Actor* InActor, const string& InPath, const string& InName);
public:
	static map<string, vector<MeshData>> MeshMap;
};

}