#pragma once
#include "D3D12Core/EnginePch.h"
#include "D3D12Utils.h"
#include "GraphicsCommonD3D12.h"
#include "ConstantBuffers.h"

#include <DirectXTexEXR.h> // EXR 형식 HDRI 읽기
#include <algorithm>
#include <cctype>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <execution>
#include <fp16.h>
#include <iostream>
#include <string>

#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include "Camera.h"
#include "GraphcisDescriptorHeap.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Ray;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;

using namespace std;
using namespace DirectX;

static const UINT FrameCount = 2;
static const UINT NumContexts = 2;

class GraphicsCommandQueue;
class SwapChain;
class RenderTargetGroup;
class DAppBase
{
public:
	int Run();

	void Init();
	void Update(float dt);
	void Render();

public:
	ComPtr<ID3D12Device> GetDevice() { return m_device; }
	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return m_graphcisCmdQueue; }
	ComPtr<ID3D12Resource> GetRenderTarget(int Index) { return m_renderTargets[Index]; }
private:
	void LoadPipeline();
	void LoadAssets();

	void InitCubemaps(wstring basePath, wstring envFilename,
		wstring specularFilename, wstring irradianceFilename,
		wstring brdfFilename);

	void SetMainViewport();
	void SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList);

	virtual void UpdateLights(float dt);
	void UpdateGlobalConstants(const float& dt, const Vector3& eyeWorld,
		const Matrix& viewRow, const Matrix& projRow,
		const Matrix& refl = Matrix());

	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return m_rtGroups[static_cast<uint8>(type)]; }
private:
// 그래픽스 관련
	ComPtr<ID3D12Device> m_device;
	shared_ptr<SwapChain> m_swapChain;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	GraphicsDescriptorHeap m_descriptorHeap;
	shared_ptr<GraphicsCommandQueue> m_graphcisCmdQueue;

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	HWND m_mainWindow;
	int m_screenWidth;
	int m_screenHeight;

	DConstantBuffer<GlobalConstants> m_globalConstBuffer;

	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_envSRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_specularSRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_irradianceSRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_brdfSRV;

	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> m_rtGroups;

// 컨텐츠 관련
	//vector<shared_ptr<Object>> m_objectList;
	Camera m_camera;
	bool m_keyPressed[256] = {
	false,
	};

};
}
