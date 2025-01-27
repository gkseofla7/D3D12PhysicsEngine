#include "AnimHelper2.h"
#include <future>
#include "ThreadPool.h"
#include "DSkinnedMeshModel2.h"
#include "../D3D12Core/StructuredBuffer2.h"
#include "Actor2.h"
namespace dengine {

AnimationData GetAnimationFromFile(string path, string name)
{
	auto [_, ani] =
		dengine::GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}

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
	m_animStateToAnim[inModelId].insert({ inState,inAnimName});
}

bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState)
{
	bool bInit;
	return LoadAnimation(inModel, inState, bInit);
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState, bool& bInit)
{
	bInit = false;
	int modelId = inModel->GetModelId();
	const string& path = m_pathMap[modelId];
	const string& animationName = m_animStateToAnim[modelId][inState];
	
	{
		std::unique_lock<std::shared_mutex> lock(mtx);
		m_animDatas.insert(std::make_pair( modelId, AnimationBlock()));
	}

	AnimationBlock& animBlock = m_animDatas[modelId];
	{
		std::unique_lock<std::shared_mutex> lock(m_animDatas[modelId].mtx);
		if (animBlock.AniData.clipMaps.find(inState) == animBlock.AniData.clipMaps.end())
		{
			// 애니메이션이 로드됐다면
			if (animBlock.Loaders.find(inState) != animBlock.Loaders.end() && animBlock.Loaders[inState]._Is_ready())
			{
				if (animBlock.IsFirstSetting == true)
				{
					animBlock.AniData = animBlock.Loaders[inState].get();
					animBlock.AniData.clipMaps[inState] = std::move(animBlock.AniData.clips.front());
					animBlock.IsFirstSetting = false;
				}
				else
				{
					AnimationData AniData = animBlock.Loaders[inState].get();
					animBlock.AniData.clipMaps[inState] = std::move(AniData.clips.front());
				}
				animBlock.Loaders.erase(inState);
			}
			// 아직 애니메이션 로드 시작을 안한경우
			else if (animBlock.Loaders.find(inState) == animBlock.Loaders.end())
			{
				hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
				std::future<AnimationData> Loader;
				Loader = tPool.EnqueueJob(GetAnimationFromFile, path, animationName);
				animBlock.Loaders.insert({ inState,std::move(Loader) });
				return false;
			}
			else
			{
				return false;
			}
		}
	}

	if (inModel->m_boneTransforms->GetCpu().size() != animBlock.AniData.clipMaps[inState].keys.size())
	{
		bInit = true;
		inModel->m_boneTransforms->GetCpu().resize(animBlock.AniData.clipMaps[inState].keys.size());
		// 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
		for (int i = 0; i < animBlock.AniData.clipMaps[inState].keys.size(); i++)
			inModel->m_boneTransforms->GetCpu()[i] = Matrix();
		inModel->m_boneTransforms->Init(); // GPU Buffer 사이즈 재할당
	}
	return true;
}
bool AnimHelper::UpdateAnimation(Actor* InActor, string inState, int frame, int type)
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

	int modelId = skinnedMeshModel->GetModelId();
	AnimationBlock& animBlock = m_animDatas[modelId];
	skinnedMeshModel->SetMaxFrame(animBlock.AniData.clipMaps[inState].keys[0].size());

	{
		std::shared_lock<std::shared_mutex> lock(m_animDatas[modelId].mtx);
		vector<Matrix> boneTransform;
		boneTransform.resize(m_animDatas[modelId].AniData.boneTransforms.size());
		Matrix& rootTransformRef = skinnedMeshModel->GetAccumulatedRootTransform();
		m_animDatas[modelId].AniData.GetBoneTransform(InActor->GetObjectId(), inState, frame, rootTransformRef, boneTransform, bInit, type);

		for (int i = 0; i < skinnedMeshModel->m_boneTransforms->GetCpu().size(); i++) {
			skinnedMeshModel->m_boneTransforms->GetCpu()[i] =
				m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]).Transpose();
			if (i == 0)
			{
				skinnedMeshModel->SetAccumulatedRootTransformToLocal(m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]));
			}
		}
	}

	return true;
}

}