#include <future>

#include "ThreadPool.h"
#include "AnimHelper.h"
#include "DSkinnedMeshModel.h"
#include "Actor.h"
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
void AnimHelper::AddAnimPath(int InModelId, string InPathName)
{
	m_pathMap[InModelId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int InModelId,string InState, string InAnimName)
{
	// 내 생각엔 단 한번 해주는게
	m_animStateToAnim[InModelId].insert({ InState,InAnimName});
}
AnimationData GetAnimationFromFile(string path,string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* InModel, string InState)
{
	bool bInit;
	return LoadAnimation(InModel, InState, bInit);
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* InModel, string InState, bool& bInit)
{
	if (bInitialize == false)
	{
		return false;
	}
	bInit = false;
	//비동기 로딩하도록 한다.
	int ModelId = InModel->m_modelId;
	const string& path = m_pathMap[ModelId];
	const string& name = m_animStateToAnim[ModelId][InState];
	
	mtx.lock();
	AnimationBlock& AnimBlock = m_animDatas[ModelId];
	mtx.unlock();

	std::unique_lock<std::mutex> lock(AnimBlock.mtx); // 락을 걸기
	if (AnimBlock.AniData.clipMaps.find(InState) == AnimBlock.AniData.clipMaps.end())
	{
		if (AnimBlock.Loaders.find(InState) != AnimBlock.Loaders.end() && AnimBlock.Loaders[InState]._Is_ready())
		{
			if (AnimBlock.IsFirstSetting == true)
			{
				AnimBlock.AniData = AnimBlock.Loaders[InState].get();
				AnimBlock.AniData.clipMaps[InState] = std::move(AnimBlock.AniData.clips.front());
				AnimBlock.IsFirstSetting = false;
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
	if (InModel->m_boneTransforms.m_cpu.size() != AnimBlock.AniData.clipMaps[InState].keys.size())
	{
		bInit = true;
		InModel->m_boneTransforms.m_cpu.resize(AnimBlock.AniData.clipMaps[InState].keys.size());
		// 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
		for (int i = 0; i < AnimBlock.AniData.clipMaps[InState].keys.size(); i++)
			InModel->m_boneTransforms.m_cpu[i] = Matrix();
		InModel->m_boneTransforms.Initialize(m_device);
	}

	return true;
}
bool AnimHelper::UpdateAnimation(Actor* InActor, string InState,
	int frame, int type)
{ 
	if (bInitialize == false)
	{
		return false;
	}
	std::shared_ptr<DSkinnedMeshModel> SkinnedMeshModel = std::dynamic_pointer_cast<DSkinnedMeshModel>(InActor->GetModel());
	if (SkinnedMeshModel == nullptr)
	{
		return false;
	}
	bool bInit = false;
	if (LoadAnimation(SkinnedMeshModel.get(), InState, bInit) == false)
	{
		return false;
	}
	if (frame == 0 && m_actorAnimState[InActor->GetObjectId()] != InState)
	{
		SkinnedMeshModel->IntegrateRootTransformToWorldTransform(m_context);
	}
	m_actorAnimState[InActor->GetObjectId()] = InState;

	int ModelId = SkinnedMeshModel->m_modelId;
	AnimationBlock& AnimBlock = m_animDatas[ModelId];
	SkinnedMeshModel->m_maxFrame = AnimBlock.AniData.clipMaps[InState].keys[0].size();
	vector<Matrix> BoneTransform;
	BoneTransform.resize(m_animDatas[ModelId].AniData.boneTransforms.size());
	m_animDatas[ModelId].AniData.GetBoneTransform(InActor->GetObjectId(),InState, frame, SkinnedMeshModel->GetAccumulatedRootTransform(), BoneTransform, bInit, type);
	for (int i = 0; i < SkinnedMeshModel->m_boneTransforms.m_cpu.size(); i++) {
		SkinnedMeshModel->m_boneTransforms.m_cpu[i] =
			m_animDatas[ModelId].AniData.GetAnimationTransform(i, BoneTransform[i]).Transpose();
		if (i == 0)
		{
			SkinnedMeshModel->SetAccumulatedRootTransformToLocal( m_animDatas[ModelId].AniData.GetAnimationTransform(i, BoneTransform[i]));
		}
	}
	SkinnedMeshModel->m_boneTransforms.Upload(m_context);
	return true;
}

}