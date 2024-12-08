#include "Engine.h"
#include "Device.h"
#include "ConstantBuffer.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "RootSignature.h"
#include "Shader.h"
#include "GraphicsPipelineState.h"
#include "GraphicsPSO.h"
#include "Samplers.h"

namespace hlab {
void Engine::Init(const WindowInfo& info)
{
	m_window = info;	
	InitGraphics();
	InitGlobalBuffer();
	//CreateConstantBuffer(CBV_REGISTER::b0, sizeof(LightParams), 1);
	//CreateConstantBuffer(CBV_REGISTER::b1, sizeof(TransformParams), 256);
	//CreateConstantBuffer(CBV_REGISTER::b2, sizeof(MaterialParams), 256);

	CreateRenderTargetGroups();

	ResizeWindow(info.width, info.height);

	//GET_SINGLE(Input)->Init(info.hwnd);
	//GET_SINGLE(Timer)->Init();
	//GET_SINGLE(Resources)->Init();
	
	// TODO. CommandList Close 및 실행 필요
}

void Engine::InitGraphics()
{
	// 그려질 화면 크기를 설정
	m_viewport = { 0, 0, static_cast<FLOAT>(m_window.width), static_cast<FLOAT>(m_window.height), 0.0f, 1.0f };
	m_scissorRect = CD3DX12_RECT(0, 0, m_window.width, m_window.height);

	m_device = std::make_shared<Device>();
	m_device->Init();

	m_swapChain = std::make_shared< SwapChain>();
	m_swapChain->Init(m_window, m_device->GetDevice(), m_device->GetDXGI(), m_graphicsCmdQueue->GetCmdQueue());

	m_graphicsCmdQueue = std::make_shared<GraphicsCommandQueue>();
	m_graphicsCmdQueue->Init(m_device->GetDevice(), m_swapChain);

	m_graphicsDescHeap = std::make_shared< GraphicsDescriptorHeap>();
	m_graphicsDescHeap->Init(256);// 흠.. Frame 개수만큼만 넣어도 되지않을까 싶긴한데

	m_shader = std::make_shared<Shader>();
	m_shader->Init();

	m_rootSignature = std::make_shared<RootSignature>();
	m_rootSignature->Init();

	m_graphicsPipelineState = std::make_shared< GraphicsPipelineState>();
	m_graphicsPipelineState->Init();

	m_defaultGraphicsPSO = std::make_shared< GraphicsPSO2>();
	m_defaultGraphicsPSO->Init(m_rootSignature->GetGraphicsRootSignature(), m_graphicsPipelineState->GetDefaultPipelineState());

	m_samplers = std::make_shared<Samplers>();
	m_samplers->Init();
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(3, m_samplers->GetDescHeap()->GetGPUDescriptorHandleForHeapStart());

}
void Engine::InitGlobalBuffer()
{
	m_globalConstsBuffer.Init(CBV_REGISTER::b0, 3);

	m_envTex = std::make_shared<Texture>();
	m_irradianceTex = std::make_shared<Texture>();
	m_specularTex = std::make_shared<Texture>();
	m_brdfTex = std::make_shared<Texture>();

	// 조명 설정
	{
		GlobalConstants& globalConstsCPU =  m_globalConstsBuffer.GetCPU();
		// 조명 0은 고정
		globalConstsCPU.lights[0].radiance = Vector3(5.0f);
		globalConstsCPU.lights[0].position = Vector3(0.0f, 1.5f, 1.1f);
		globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
		globalConstsCPU.lights[0].spotPower = 3.0f;
		globalConstsCPU.lights[0].radius = 0.04f;
		globalConstsCPU.lights[0].type =
			LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

		// 조명 1의 위치와 방향은 Update()에서 설정
		globalConstsCPU.lights[1].radiance = Vector3(5.0f);
		globalConstsCPU.lights[1].spotPower = 3.0f;
		globalConstsCPU.lights[1].fallOffEnd = 20.0f;
		globalConstsCPU.lights[1].radius = 0.02f;
		globalConstsCPU.lights[1].type =
			LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

		// 조명 2는 꺼놓음
		globalConstsCPU.lights[2].type = LIGHT_OFF;
	}
}

void Engine::InitCubemaps(wstring basePath, wstring envFilename,
	wstring specularFilename, wstring irradianceFilename,
	wstring brdfFilename) {
	// BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
	m_envTex->Load((basePath + envFilename).c_str(), true);
	GRAPHICS_CMD_LIST->SetGraphicsRootShaderResourceView(1, m_envTex->GetTex2D()->GetGPUVirtualAddress());

	m_irradianceTex->Load((basePath + irradianceFilename).c_str(), true);
	GRAPHICS_CMD_LIST->SetGraphicsRootShaderResourceView(2, m_irradianceTex->GetTex2D()->GetGPUVirtualAddress());

	m_specularTex->Load((basePath + specularFilename).c_str(), true);
	GRAPHICS_CMD_LIST->SetGraphicsRootShaderResourceView(3, m_specularTex->GetTex2D()->GetGPUVirtualAddress());

	m_brdfTex->Load((basePath + brdfFilename).c_str(), true);
	GRAPHICS_CMD_LIST->SetGraphicsRootShaderResourceView(4, m_brdfTex->GetTex2D()->GetGPUVirtualAddress());
}

void Engine::Update()
{
	//GET_SINGLE(Input)->Update();
	//GET_SINGLE(Timer)->Update();
	//GET_SINGLE(SceneManager)->Update();
	//GET_SINGLE(InstancingManager)->ClearBuffer();

	Render();
}

void Engine::Render()
{
	RenderBegin();

	//GET_SINGLE(SceneManager)->Render();

	RenderEnd();
}

void Engine::RenderBegin()
{

	// 공통으로 사용할 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(
		1, m_globalConstsBuffer->GetGpuVirtualAddress(m_swapChain->GetBackBufferIndex()));

	m_graphicsCmdQueue->RenderBegin();
}

void Engine::RenderEnd()
{
	m_graphicsCmdQueue->RenderEnd();
}

void Engine::ResizeWindow(int32 width, int32 height)
{
	m_window.width = width;
	m_window.height = height;

	RECT rect = { 0, 0, width, height };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	::SetWindowPos(m_window.hwnd, 0, 100, 100, width, height, 0);
}


//void Engine::CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count)
//{
//	uint8 typeInt = static_cast<uint8>(reg);
//	assert(m_constantBuffers.size() == typeInt);
//
//	shared_ptr<ConstantBuffer> buffer = std::make_shared<ConstantBuffer>();
//	buffer->Init(reg, bufferSize, count);
//	m_constantBuffers.push_back(buffer);
//}


void Engine::CreateRenderTargetGroups()
{
	// DepthStencil
	shared_ptr<Texture> dsTexture = std::make_shared<Texture>();
	dsTexture->Create(DXGI_FORMAT_D32_FLOAT, m_window.width, m_window.height,
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	// SwapChain Group
	{
		vector<RenderTarget> rtVec(SWAP_CHAIN_BUFFER_COUNT);

		for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			ComPtr<ID3D12Resource> resource;
			m_swapChain->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&resource));
			rtVec[i].target = std::make_shared<Texture>();
			rtVec[i].target->CreateFromResource(resource);
		}

		m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)] = std::make_shared<RenderTargetGroup>();
		m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTexture);
	}

	//// Shadow Group
	//{
	//	vector<RenderTarget> rtVec(RENDER_TARGET_SHADOW_GROUP_MEMBER_COUNT);

	//	rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"ShadowTarget",
	//		DXGI_FORMAT_R32_FLOAT, 4096, 4096,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	shared_ptr<Texture> shadowDepthTexture = GET_SINGLE(Resources)->CreateTexture(L"ShadowDepthStencil",
	//		DXGI_FORMAT_D32_FLOAT, 4096, 4096,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)] = make_shared<RenderTargetGroup>();
	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)]->Create(RENDER_TARGET_GROUP_TYPE::SHADOW, rtVec, shadowDepthTexture);
	//}

	//// Deferred Group
	//{
	//	vector<RenderTarget> rtVec(RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT);

	//	rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"PositionTarget",
	//		DXGI_FORMAT_R32G32B32A32_FLOAT, m_window.width, m_window.height,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"NormalTarget",
	//		DXGI_FORMAT_R32G32B32A32_FLOAT, m_window.width, m_window.height,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	rtVec[2].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseTarget",
	//		DXGI_FORMAT_R8G8B8A8_UNORM, m_window.width, m_window.height,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)] = make_shared<RenderTargetGroup>();
	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)]->Create(RENDER_TARGET_GROUP_TYPE::G_BUFFER, rtVec, dsTexture);
	//}

	//// Lighting Group
	//{
	//	vector<RenderTarget> rtVec(RENDER_TARGET_LIGHTING_GROUP_MEMBER_COUNT);

	//	rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseLightTarget",
	//		DXGI_FORMAT_R8G8B8A8_UNORM, m_window.width, m_window.height,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"SpecularLightTarget",
	//		DXGI_FORMAT_R8G8B8A8_UNORM, m_window.width, m_window.height,
	//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)] = make_shared<RenderTargetGroup>();
	//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)]->Create(RENDER_TARGET_GROUP_TYPE::LIGHTING, rtVec, dsTexture);
	//}
}
}
