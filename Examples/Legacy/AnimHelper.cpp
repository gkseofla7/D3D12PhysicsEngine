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
void AnimHelper::AddAnimPath(int inModelId, string InPathName)
{
	m_pathMap[inModelId] = InPathName;
}
void AnimHelper::AddAnimStateToAnim(int inModelId,string inState, string inAnimName)
{
	// �� ������ �� �ѹ� ���ִ°�
	m_animStateToAnim[inModelId].insert({ inState,inAnimName});
}
AnimationData GetAnimationFromFile(string path,string name)
{
	auto [_, ani] =
		GeometryGenerator::ReadAnimationFromFile(path, name);
	return ani;
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState)
{
	bool bInit;
	return LoadAnimation(inModel, inState, bInit);
}
bool AnimHelper::LoadAnimation(DSkinnedMeshModel* inModel, string inState, bool& bInit)
{
	if (bInitialize == false)
	{
		return false;
	}
	bInit = false;
	//�񵿱� �ε��ϵ��� �Ѵ�.
	int modelId = inModel->m_modelId;
	const string& path = m_pathMap[modelId];
	const string& name = m_animStateToAnim[modelId][inState];
	
	mtx.lock();
	AnimationBlock& animBlock = m_animDatas[modelId];
	mtx.unlock();

	std::unique_lock<std::mutex> lock(animBlock.mtx); // ���� �ɱ�
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
			{ // TODO. R-Value�� �Ѱ��ִ°�,,
				AnimationData AniData = animBlock.Loaders[inState].get();
				animBlock.AniData.clipMaps[inState] = std::move(AniData.clips.front());
			}
			animBlock.Loaders.erase(inState);
		}
		else if (animBlock.Loaders.find(inState) == animBlock.Loaders.end())
		{
			ThreadPool& tPool = ThreadPool::getInstance();
			std::future<AnimationData> Loader;
			Loader = tPool.EnqueueJob(GetAnimationFromFile, path, name);
			animBlock.Loaders.insert({ inState,std::move(Loader) });
			return false;
		}
		else
		{
			return false;
		}
	}
	if (inModel->m_boneTransforms->m_cpu.size() != animBlock.AniData.clipMaps[inState].keys.size())
	{
		bInit = true;
		inModel->m_boneTransforms->m_cpu.resize(animBlock.AniData.clipMaps[inState].keys.size());
		// ����: ��� keys() ������ �������� ���� ���� �ֽ��ϴ�.
		for (int i = 0; i < animBlock.AniData.clipMaps[inState].keys.size(); i++)
			inModel->m_boneTransforms->m_cpu[i] = Matrix();
		inModel->m_boneTransforms->Initialize(m_device);
	}

	return true;
}
bool AnimHelper::UpdateAnimation(Actor* InActor, string inState,
	int frame, int type)
{ 
	if (bInitialize == false)
	{
		return false;
	}
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
		skinnedMeshModel->IntegrateRootTransformToWorldTransform(m_context);
	}
	m_actorAnimState[InActor->GetObjectId()] = inState;

	int modelId = skinnedMeshModel->m_modelId;
	AnimationBlock& animBlock = m_animDatas[modelId];
	skinnedMeshModel->m_maxFrame = animBlock.AniData.clipMaps[inState].keys[0].size();
	vector<Matrix> boneTransform;
	boneTransform.resize(m_animDatas[modelId].AniData.boneTransforms.size());
	m_animDatas[modelId].AniData.GetBoneTransform(InActor->GetObjectId(),inState, frame, skinnedMeshModel->GetAccumulatedRootTransform(), boneTransform, bInit, type);
	for (int i = 0; i < skinnedMeshModel->m_boneTransforms->m_cpu.size(); i++) {
		skinnedMeshModel->m_boneTransforms->m_cpu[i] =
			m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]).Transpose();
		if (i == 0)
		{
			skinnedMeshModel->SetAccumulatedRootTransformToLocal( m_animDatas[modelId].AniData.GetAnimationTransform(i, boneTransform[i]));
		}
	}
	skinnedMeshModel->m_boneTransforms->Upload(m_context);
	return true;
}

}