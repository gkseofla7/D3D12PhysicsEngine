#include "AnimHelper2.h"
#include <future>

#include "../ThreadPool.h"

#include "DSkinnedMeshModel2.h"
#include "StructuredBuffer2.h"
#include "Actor2.h"
namespace dengine {

AnimHelper& AnimHelper::GetInstance()
{
	static AnimHelper helper;
	return helper;
}
void AnimHelper::Initialize()
{
	m_animStateToAnim.clear();
	m_pathMap.clear();
	m_animDatas.clear();
}
void AnimHelper::AddAnimPath(int inModelId, string InPathName)
{
	m_pathMap[inModelId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int inModelId,string inState, string inAnimName)
{
	// 내 생각엔 단 한번 해주는게
	m_animStateToAnim[inModelId].insert({ inState,inAnimName});
}
dengine::AnimationData GetAnimationFromFile2(string path,string name)
{
	auto [_, ani] =
		dengine::GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState)
{
	bool bInit;
	return LoadAnimation(inModel, inState, bInit);
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState, bool& bInit)
{
	bInit = false;
	int modelId = inModel->m_modelId;
	const string& path = m_pathMap[modelId];
	const string& name = m_animStateToAnim[modelId][inState];
	
	mtx.lock();
	AnimationBlock& animBlock = m_animDatas[modelId];
	mtx.unlock();

	std::unique_lock<std::mutex> lock(animBlock.mtx); // 락을 걸기
	if (animBlock.AniData.clipMaps.find(inState) == animBlock.AniData.clipMaps.end())
	{
		if (animBlock.Loaders.find(inState) != animBlock.Loaders.end() && animBlock.Loaders[inState]._Is_ready())
		{
			if (animBlock.IsFirstSetting == true)
			{
				animBlock.AniData = animBlock.Loaders[inState].get();
				animBlock.AniData.clipMaps[inState] = std::move(animBlock.AniData.clips.front());
				animBlock.IsFirstSetting = false;
			}
			else
			{ // TODO. R-Value로 넘겨주는게,,
				AnimationData AniData = animBlock.Loaders[inState].get();
				animBlock.AniData.clipMaps[inState] = std::move(AniData.clips.front());
			}
			animBlock.Loaders.erase(inState);
		}
		else if (animBlock.Loaders.find(inState) == animBlock.Loaders.end())
		{
			hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
			std::future<AnimationData> Loader;
			Loader = tPool.EnqueueJob(GetAnimationFromFile2, path, name);
			animBlock.Loaders.insert({ inState,std::move(Loader) });
			return false;
		}
		else
		{
			return false;
		}
	}


	bInit = true;
	if (inModel->m_boneTransforms->GetCpu().size() != animBlock.AniData.clipMaps[inState].keys.size())
	{
		
		inModel->m_boneTransforms->GetCpu().resize(animBlock.AniData.clipMaps[inState].keys.size());
		// 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
		for (int i = 0; i < animBlock.AniData.clipMaps[inState].keys.size(); i++)
			inModel->m_boneTransforms->GetCpu()[i] = Matrix();
		inModel->m_boneTransforms->Init();
	}

	return true;
}
bool AnimHelper::UpdateAnimation(Actor* InActor, string inState,
	int frame, int type)
{ 
	std::shared_ptr<DSkinnedMeshModel> skinnedMeshModel = std::dynamic_pointer_cast<DSkinnedMeshModel>(InActor->GetModel());
	if (skinnedMeshModel == nullptr)
	{
		return false;
	}
	bool bInit = false;
	if (LoadAnimation(skinnedMeshModel.get(), inState, bInit) == false)
	{
		return false;
	}
	if (frame == 0 && m_actorAnimState[InActor->GetObjectId()] != inState)
	{
		skinnedMeshModel->IntegrateRootTransformToWorldTransform();
	}
	m_actorAnimState[InActor->GetObjectId()] = inState;

	int modelId = skinnedMeshModel->m_modelId;
	AnimationBlock& animBlock = m_animDatas[modelId];
	skinnedMeshModel->m_maxFrame = animBlock.AniData.clipMaps[inState].keys[0].size();
	vector<Matrix> boneTransform;
	boneTransform.resize(m_animDatas[modelId].AniData.boneTransforms.size());
	m_animDatas[modelId].AniData.GetBoneTransform(InActor->GetObjectId(),inState, frame, skinnedMeshModel->GetAccumulatedRootTransform(), boneTransform, bInit, type);
	for (int i = 0; i < skinnedMeshModel->m_boneTransforms->GetCpu().size(); i++) {
		skinnedMeshModel->m_boneTransforms->GetCpu()[i] =
			m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]).Transpose();
		if (i == 0)
		{
			skinnedMeshModel->SetAccumulatedRootTransformToLocal( m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]));
		}
	}
	skinnedMeshModel->m_boneTransforms->Upload();
	return true;
}

}