#pragma once

#include "EnginePch.h"
//#include "Mesh.h"
//#include "Shader.h"

#include "TableDescriptorHeap.h"
#include "RenderTargetGroup.h"
namespace hlab {
class Device;
class ConstantBuffer;
class CommandQueue;
class RootSignature;
class GraphicsCommandQueue;
class SwapChain;
class RenderTargetGroup;
class Shader;
class GraphicsPipelineState;
class GraphicsPSO;
class Texture;
class Samplers;
class Engine
{
public:
	void Init(const WindowInfo& info);

	void InitGraphics();
	void InitGlobalBuffer();
	void InitCubemaps(wstring basePath, wstring envFilename,
		wstring specularFilename, wstring irradianceFilename,
		wstring brdfFilename);
	void Update();

public:
	const WindowInfo& GetWindow() { return m_window; }
	shared_ptr<Device> GetDevice() { return m_device; }
	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return m_graphicsCmdQueue; }
	shared_ptr<SwapChain> GetSwapChain() { return m_swapChain; }
	shared_ptr<RootSignature> GetRootSignature() { return m_rootSignature; }
	shared_ptr<GraphicsDescriptorHeap> GetGraphicsDescHeap() { return m_graphicsDescHeap; }
	shared_ptr<Shader> GetShader() { return m_shader; }
	shared_ptr<Samplers> GetSamples() { return m_samplers; }

	shared_ptr<ConstantBuffer> GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return m_constantBuffers[static_cast<uint8>(type)]; }
	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return m_rtGroups[static_cast<uint8>(type)]; }

public:
	void Render();
	void RenderBegin();
	void RenderEnd();

	void ResizeWindow(int32 width, int32 height);

private:
	//void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();

private:
	// 그려질 화면 크기 관련
	WindowInfo		m_window;
	D3D12_VIEWPORT	m_viewport = {};
	D3D12_RECT		m_scissorRect = {};

	shared_ptr<Device> m_device;// = make_shared<Device>();
	shared_ptr<SwapChain> m_swapChain;// = make_shared<SwapChain>();
	shared_ptr<GraphicsCommandQueue> m_graphicsCmdQueue;// = make_shared<GraphicsCommandQueue>();
	shared_ptr<GraphicsDescriptorHeap> m_graphicsDescHeap;// = make_shared<GraphicsDescriptorHeap>();
	shared_ptr<Shader> m_shader;
	shared_ptr<RootSignature> m_rootSignature;// = make_shared<RootSignature>();
	shared_ptr<GraphicsPipelineState> m_graphicsPipelineState;
	shared_ptr<Samplers> m_samplers;

	shared_ptr<GraphicsPSO2> m_defaultGraphicsPSO;

	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> m_rtGroups;

	ConstantBuffer<GlobalConstants> m_globalConstsBuffer;

	shared_ptr<Texture> m_envTex;
	shared_ptr<Texture> m_irradianceTex;
	shared_ptr<Texture> m_specularTex;
	shared_ptr<Texture> m_brdfTex;

};
}


