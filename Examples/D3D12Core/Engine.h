#pragma once

#include "EnginePch.h"
//#include "Mesh.h"
//#include "Shader.h"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include "TableDescriptorHeap.h"
#include "RenderTargetGroup.h"
#include "../Camera.h"
namespace dengine {
class Device;
class CommandQueue;
class RootSignature;
class GraphicsCommandQueue;
class SwapChain;
class Shader;
class GraphicsPipelineState;
class GraphicsPSO;
class Texture;
class Samplers;
class DModel2;
class DSkinnedMeshModel2;
template <typename T_CONSTS> class ConstantBuffer2;
struct GlobalConstants2;
class Engine
{
public:
	int Run();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	Engine(){}
	~Engine();
	void Init(const WindowInfo& info);

	bool InitMainWindow();
	bool InitGUI();
	void InitGraphics();
	void InitPSO();
	void InitGlobalBuffer();
	void InitCubemaps(wstring basePath, wstring envFilename,
		wstring specularFilename, wstring irradianceFilename,
		wstring brdfFilename);
	virtual bool InitScene();
	void Update(float dt);

	float GetAspectRatio() const;

public:
	const WindowInfo& GetWindow() { return m_window; }
	shared_ptr<Device> GetDevice() { return m_device; }
	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return m_graphicsCmdQueue; }
	shared_ptr<SwapChain> GetSwapChain() { return m_swapChain; }
	shared_ptr<RootSignature> GetRootSignature() { return m_rootSignature; }
	shared_ptr<GraphicsDescriptorHeap> GetGraphicsDescHeap() { return m_graphicsDescHeap; }
	shared_ptr<Shader> GetShader() { return m_shader; }
	shared_ptr<Samplers> GetSamples() { return m_samplers; }

	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return m_rtGroups[static_cast<uint8>(type)]; }

public:
	void Render();
	void RenderBegin();
	void RenderEnd();

	void SetMainViewport();
	void ResizeWindow(int32 width, int32 height);

private:
	void UpdateGlobalConstants(const float& dt, const Vector3& eyeWorld,
		const Matrix& viewRow,
		const Matrix& projRow, const Matrix& refl);

	//void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();
	virtual void OnMouseMove(int mouseX, int mouseY);
	virtual void OnMouseClick(int mouseX, int mouseY);
private:
	// 그래픽스 관련
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

	shared_ptr<GraphicsPSO> m_defaultGraphicsPSO;
	shared_ptr<GraphicsPSO> m_skyboxGraphicsPSO;
public:
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> m_rtGroups;

	shared_ptr<ConstantBuffer2<GlobalConstants2>> m_globalConstsBuffer;

	shared_ptr<Texture> m_envTex;
	shared_ptr<Texture> m_irradianceTex;
	shared_ptr<Texture> m_specularTex;
	shared_ptr<Texture> m_brdfTex;

	// 윈도우 관련
	WindowInfo		m_window;

	float m_mouseNdcX = 0.0f;
	float m_mouseNdcY = 0.0f;
	float m_wheelDelta = 0.0f;
	int m_mouseX = -1;
	int m_mouseY = -1;

	bool m_leftButton = false;
	bool m_rightButton = false;
	bool m_dragStartFlag = false;

	bool m_keyPressed[256] = {false,};

	// 컨텐츠 관련
	hlab::Camera m_camera;
	// DaerimGTA
	shared_ptr<DSkinnedMeshModel2> m_activeModel;
	shared_ptr<DModel2> m_skybox;
	// EDaerimGTA
};
}


