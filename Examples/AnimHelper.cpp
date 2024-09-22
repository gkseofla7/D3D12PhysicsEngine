#include <future>

#include "ThreadPool.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
namespace hlab {

AnimHelper& AnimHelper::GetInstance()
{
	static AnimHelper helper;
	return helper;
}
void AnimHelper::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	if (bInitialize)
	{
		return;
	}
	m_device = device;
	m_context = context;

	m_animStateToAnim.clear();
	m_pathMap.clear();
	m_animDatas.clear();

	bInitialize = true;
}
void AnimHelper::AddAnimPath(int InActorId, string InPathName)
{
	m_pathMap[InActorId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int InActorId,string InState, string InAnimName)
{
	// 내 생각엔 단 한번 해주는게
	m_animStateToAnim[InActorId].insert({ InState,InAnimName});
}
AnimationData GetAnimationFromFile(string path,string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* InActor, string InState)
{
	bool bInit;
	return LoadAnimation(InActor, InState, bInit);
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* InActor, string InState, bool& bInit)
{
	if (bInitialize == false)
	{
		return false;
	}
	bInit = false;
	//비동기 로딩하도록 한다.
	int ActorId = InActor->m_modelId;
	const string& path = m_pathMap[ActorId];
	const string& name = m_animStateToAnim[ActorId][InState];
	
	mtx.lock();
	AnimationBlock& AnimBlock = m_animDatas[ActorId];
	mtx.unlock();

	std::unique_lock<std::mutex> lock(AnimBlock.mtx); // 락을 걸기
	if (AnimBlock.AniData.clipMaps.find(InState) == AnimBlock.AniData.clipMaps.end())
	{
		if (AnimBlock.Loaders.find(InState) != AnimBlock.Loaders.end() && AnimBlock.Loaders[InState]._Is_ready())
		{
			bInit = true;
			if (AnimBlock.IsFirstSetting == true)
			{
				AnimBlock.IsFirstSetting = false;
				AnimBlock.AniData = AnimBlock.Loaders[InState].get();
				AnimBlock.AniData.clipMaps[InState] = std::move(AnimBlock.AniData.clips.front());
				InActor->m_boneTransforms.m_cpu.resize(AnimBlock.AniData.clipMaps[InState].keys.size());
				// 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
				for (int i = 0; i < AnimBlock.AniData.clipMaps[InState].keys.size(); i++)
					InActor->m_boneTransforms.m_cpu[i] = Matrix();
				InActor->m_boneTransforms.Initialize(m_device);
			}
			else
			{ // TODO. R-Value로 넘겨주는게,,
				AnimationData AniData = AnimBlock.Loaders[InState].get();
				AnimBlock.AniData.clipMaps[InState] = std::move(AniData.clips.front());
			}
			AnimBlock.Loaders.erase(InState);
		}
		else if (AnimBlock.Loaders.find(InState) == AnimBlock.Loaders.end())
		{
			ThreadPool& tPool = ThreadPool::getInstance();
			std::future<AnimationData> Loader;
			Loader = tPool.EnqueueJob(GetAnimationFromFile, path, name);
			AnimBlock.Loaders.insert({ InState,std::move(Loader) });
			return false;
		}
		else
		{
			return false;
		}
	}
	return true;
}
bool AnimHelper::UpdateAnimation(DSkinnedMeshModel* InActor, string InState,
	int frame, int type)
{ 
	if (bInitialize == false)
	{
		return false;
	}
	bool bInit = false;
	if (LoadAnimation(InActor, InState, bInit) == false)
	{
		return false;
	}
	int ActorId = InActor->m_modelId;
	AnimationBlock& AnimBlock = m_animDatas[ActorId];
	InActor->m_maxFrame = AnimBlock.AniData.clipMaps[InState].keys[0].size();
	vector<Matrix> BoneTransform;
	BoneTransform.resize(m_animDatas[ActorId].AniData.boneTransforms.size());
	m_animDatas[ActorId].AniData.GetBoneTransform(InState, frame, InActor->m_accumulatedRootTransform, BoneTransform, bInit, type);
	for (int i = 0; i < InActor->m_boneTransforms.m_cpu.size(); i++) {
		InActor->m_boneTransforms.m_cpu[i] = 
			m_animDatas[ActorId].AniData.GetAnimationTransform(i, BoneTransform[i]).Transpose();
		if (i == 0)
		{
			InActor->m_accumulatedRootTransformToLocal = m_animDatas[ActorId].AniData.GetAnimationTransform(i, BoneTransform[i]);
		}
	}
	 
	InActor->m_boneTransforms.Upload(m_context);
	return true;
}

}