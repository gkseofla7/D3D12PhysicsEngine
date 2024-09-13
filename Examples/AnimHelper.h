#pragma once
#include <map>
#include <string>
#include <future>
#include "GeometryGenerator.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"

namespace hlab {
using namespace std;
class DSkinnedMeshModel;
struct AnimationBlock
{
	//map<int, string > AnimStateToAnimName;
	string PathName;
	AnimationData AniData;
	std::future<AnimationData> Loader;
	bool IsLoading = false;
	bool IsFirstSetting = true;
};
class AnimHelper
{
private:
	AnimHelper() {};
public:
	static AnimHelper& GetInstance();
	void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
	void AddAnimPath(int InActorId, string InPathName);
	void AddAnimStateToAnim(int InActorId, int InState, string InAnimName);
	bool UpdateAnimation(DSkinnedMeshModel* InActor, int InState,
		int frame, int type = 0);
private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;

	map<int ,map<int, string>> m_animStateToAnim;
	map<int, string> m_pathMap;
	map<int, AnimationBlock> m_animDatas;
	
	bool bInitialize = false;
};

}