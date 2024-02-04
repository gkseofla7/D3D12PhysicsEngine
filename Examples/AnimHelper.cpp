#include <future>
#include <thread>

#include "AnimHelper.h"
#include "SkeletalMeshActor.h"
#include "ThreadPool.h"
namespace hlab {
void AnimHelper::AddAnimPath(int InActorId, string InPathName)
{
	m_pathMap[InActorId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int InActorId,int InState, string InAnimName)
{
	// 내 생각엔 단 한번 해주는게
	m_animStateToAnim[InActorId].insert({ InState,InAnimName});
}
AnimationData ReadAnimationFromFile(string path,string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool AnimHelper::UpdateAnimation(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, SkeletalMeshActor* InActor,int InState,
	int frame, int type = 0)
{
	//비동기 로딩하도록 한다.
	int ActorId = InActor->getActorId();		
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

			AnimationData AniData =AnimBlock.Loader.get();
			AnimBlock.AniData.clipMaps[InState] = AniData.clips.front();
			
		}
		else if (AnimBlock.IsLoading == false)
		{
			ThreadPool& tPool =ThreadPool::getInstance();
			AnimBlock.Loader = tPool.EnqueueJob(ReadAnimationFromFile, path, name);
			m_animDatas[ActorId].IsLoading = true;
		}
		return false;
	}
	//m_aniData[ActorId].Update(InState, frame, type);
	vector<Matrix> BoneTransform;
	m_aniData[ActorId].GetBoneTransform(InState, frame, InActor->accumulatedRootTransform, BoneTransform, type);

	for (int i = 0; i < InActor->m_boneTransforms.m_cpu.size(); i++) {
		InActor->m_boneTransforms.m_cpu[i] =
			m_aniData[ActorId].GetAnimationTransform(i, BoneTransform[i]).Transpose();
	}

	InActor->m_boneTransforms.Upload(context);
	return true;
}

}