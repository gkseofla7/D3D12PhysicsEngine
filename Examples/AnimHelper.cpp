#include <future>

#include "ThreadPool.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
namespace hlab {
	map<int, map<int, string>> AnimHelper::m_animStateToAnim;
	map<int, string> AnimHelper::m_pathMap;
	map<int, AnimationBlock> AnimHelper::m_animDatas;
void AnimHelper::AddAnimPath(int InActorId, string InPathName)
{
	m_pathMap[InActorId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int InActorId,int InState, string InAnimName)
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
bool AnimHelper::UpdateAnimation(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, DSkinnedMeshModel* InActor,int InState,
	int frame, int type)
{ 
	//비동기 로딩하도록 한다.
	int ActorId = InActor->m_modelId;
	const string& path = m_pathMap[ActorId];
	const string& name = m_animStateToAnim[ActorId][InState];
	if (m_animDatas.find(ActorId) == m_animDatas.end())
	{
		m_animDatas[ActorId] = AnimationBlock();
	}
	AnimationBlock& AnimBlock =m_animDatas[ActorId];
	if (AnimBlock.AniData.clipMaps.find(InState) == AnimBlock.AniData.clipMaps.end())
	{
		if (AnimBlock.IsLoading == true&& AnimBlock.Loader._Is_ready())
		{
			AnimBlock.IsLoading = false;
			if (AnimBlock.IsFirstSetting == true)
			{
				AnimBlock.IsFirstSetting = false;
				AnimBlock.AniData = AnimBlock.Loader.get();
				InActor->m_boneTransforms.m_cpu.resize(AnimBlock.AniData.clips.front().keys.size());
				// 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
				for (int i = 0; i < AnimBlock.AniData.clips.front().keys.size(); i++)
					InActor->m_boneTransforms.m_cpu[i] = Matrix();
				InActor->m_boneTransforms.Initialize(device);
			}
			else
			{ 
				AnimationData AniData = AnimBlock.Loader.get();
				AnimBlock.AniData.clipMaps[InState] = AniData.clips.front();
			}
		} 
		else if (AnimBlock.IsLoading == false)
		{
			ThreadPool& tPool =ThreadPool::getInstance();
			AnimBlock.Loader = tPool.EnqueueJob(GetAnimationFromFile, path, name);
			m_animDatas[ActorId].IsLoading = true;
		}
		return false;
	}   
	//m_aniData[ActorId].Update(InState, frame, type);
	InActor->m_maxFrame = AnimBlock.AniData.clips[0].keys[0].size();
	vector<Matrix> BoneTransform;
	BoneTransform.resize(m_animDatas[ActorId].AniData.boneTransforms.size());
	m_animDatas[ActorId].AniData.GetBoneTransform(InState, frame, InActor->m_accumulatedRootTransform, BoneTransform, type);
	for (int i = 0; i < InActor->m_boneTransforms.m_cpu.size(); i++) {
		InActor->m_boneTransforms.m_cpu[i] = 
			m_animDatas[ActorId].AniData.GetAnimationTransform(i, BoneTransform[i]).Transpose();
	}
	 
	InActor->m_boneTransforms.Upload(context);
	return true;
}

}