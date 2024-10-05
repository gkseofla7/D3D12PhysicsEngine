#pragma once
#include <map>
#include <string>
#include <future>
#include <unordered_set>
#include "GeometryGenerator.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"

namespace hlab {
using namespace std;
class DSkinnedMeshModel;
class Actor;
struct AnimationBlock
{
	//AnimationBlock() = default;
	//AnimationBlock(AnimationBlock&&) noexcept = default; // 이동 생성자
	//AnimationBlock& operator=(AnimationBlock&&) noexcept = default; // 이동 대입 연산자

	//// 복사 금지
	//AnimationBlock(const AnimationBlock&) = delete;
	//AnimationBlock& operator=(const AnimationBlock&) = delete;

	//map<int, string > AnimStateToAnimName;
	string PathName;
	AnimationData AniData;
	std::unordered_map<string,std::future<AnimationData>> Loaders;
	bool IsFirstSetting = true;
	std::mutex mtx;
};
class AnimHelper
{
private:
	AnimHelper() {};
public:
	static AnimHelper& GetInstance();
	void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
	void AddAnimPath(int InActorId, string InPathName);
	void AddAnimStateToAnim(int InActorId, string InState, string InAnimName);
	bool LoadAnimation(DSkinnedMeshModel* InActor, string InState);
	bool UpdateAnimation(Actor* InActor, string InState,
		int frame, int type = 0);

private:
	bool LoadAnimation(DSkinnedMeshModel* InActor, string InState, bool& bInit);
private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;

	map<int ,map<string, string>> m_animStateToAnim;
	map<int, string> m_pathMap;
	map<int, AnimationBlock> m_animDatas;
	
	std::mutex mtx;
	bool bInitialize = false;
};

}