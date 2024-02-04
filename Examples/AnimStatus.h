#pragma once
#include <map>
#include <string>
#include "GeometryGenerator.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"

namespace hlab {
using namespace std;
class SkeletalMeshActor;
struct AnimationBlock
{
	//map<int, string > AnimStateToAnimName;
	string PathName;
	AnimationData AniData;
	std::future<AnimationData> Loader;
	bool IsLoading = false;
	bool IsFirstSetting = true;
};
class AnimStatus
{
public:
	static void AddAnimPath(int InActorId, string InPathName);
	static void AddAnimStateToAnim(int InActorId, int InState, string InAnimName);
	static bool UpdateAnimation(ComPtr<ID3D11DeviceContext>& context, SkeletalMeshActor* InActor, int InState,
		int frame, int type = 0);
private:
	static map<int ,map<int, string>> m_animStateToAnim;
	static map<int, string> m_pathMap;
	static map<int, AnimationData> m_aniData;
	static map<int, std::future<AnimationData>> m_asyncLoader;
	static map<int, AnimationBlock> m_animDatas;

};

}