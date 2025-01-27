#include "Scene.h"
#include "D3D12Core/Engine.h"
#include "D3D12Core/SwapChain.h"
#include "D3D12Core/ConstantBuffer.h"

namespace dengine {




shared_ptr<Camera> Scene::GetMainCamera()
{
	if (_cameras.empty())
		return nullptr;

	return _cameras[0];
}

void Scene::Render()
{
	PushLightData();

	ClearRTV();

	// Swapchain OMSet
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->OMSetRenderTargets(1, 0);
}

void Scene::ClearRTV()
{
	// SwapChain Group ÃÊ±âÈ­
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->ClearRenderTargetView(0);
}

void Scene::PushLightData()
{
	//LightParams lightParams = {};

	//for (auto& light : _lights)
	//{
	//	const LightInfo& lightInfo = light->GetLightInfo();

	//	light->SetLightIndex(lightParams.lightCount);

	//	lightParams.lights[lightParams.lightCount] = lightInfo;
	//	lightParams.lightCount++;
	//}

	//CONST_BUFFER(CONSTANT_BUFFER_TYPE::GLOBAL)->SetGraphicsGlobalData(&lightParams, sizeof(lightParams));
}

}

