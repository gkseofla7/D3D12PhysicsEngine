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
	//AnimationBlock(AnimationBlock&&) noexcept = default; // �̵� ������
	//AnimationBlock& operator=(AnimationBlock&&) noexcept = default; // �̵� ���� ������

	//// ���� ����
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
	void AddAnimStateToAnim(int InActorId, string inState, string inAnimName);
	bool LoadAnimation(DSkinnedMeshModel* InActor, string inState);
	bool UpdateAnimation(Actor* InActor, string inState,
		int frame, int type = 0);

private:
	bool LoadAnimation(DSkinnedMeshModel* InActor, string inState, bool& bInit);
private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;

	map<int ,map<string, string>> m_animStateToAnim;
	map<int, string> m_pathMap;
	// modelId -> AnimationBlock
	map<int, AnimationBlock> m_animDatas;

	// ������ ���� �ִϸ��̼�
	map<int, string> m_actorAnimState;
	
	std::mutex mtx;
	bool bInitialize = false;
};

}