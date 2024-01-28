#include <future>
#include <thread>

#include "AnimStatus.h"
#include "SkeletalMeshActor.h"
namespace hlab {
void AnimStatus::AddAnimPath(int InActorId, string InPathName)
{
	m_pathMap[InActorId] = InPathName;
}
void AnimStatus::AddAnimStateToAnim(int InActorId,int InState, string InAnimName)
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
bool AnimStatus::UpdateAnimation(ComPtr<ID3D11DeviceContext>& context, SkeletalMeshActor* InActor,int InState,
	int frame, int type = 0)
{
	//비동기 로딩하도록 한다.
	int ActorId = InActor->getActorId();		const string& path = m_pathMap[ActorId];
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
			}

			AnimationData AniData =AnimBlock.Loader.get();
			AnimBlock.AniData.clipMaps[InState] = AniData.clips.front();
			
		}
		else if (AnimBlock.IsLoading == false)
		{
			m_animDatas[ActorId].Loader =
				std::async(std::launch::async, ReadAnimationFromFile, path, name);
			m_animDatas[ActorId].IsLoading = true;
		}
		return false;
	}
	m_aniData[ActorId].Update(InState, frame, type);

	for (int i = 0; i < InActor->m_boneTransforms.m_cpu.size(); i++) {
		InActor->m_boneTransforms.m_cpu[i] =
			m_aniData[ActorId].GetAnimationTransform(InState, i, frame).Transpose();
	}

	InActor->m_boneTransforms.Upload(context);
	return true;
}

}