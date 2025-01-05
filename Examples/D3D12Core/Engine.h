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
class DModel;
class DSkinnedMeshModel;
class Wizard;
template <typename T_CONSTS> class ConstantBuffer;
struct GlobalConstants;

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
	void UpdateLights(float dt);

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
	void RenderOpaqueObjects();
	void RenderShadowMaps();
	void PostRender();

	void RenderBegin();
	void RenderEnd();

	void SetMainViewport();
	void ResizeWindow(int32 width, int32 height);
	
	void SetPSOType(const PSOType psoType) { m_psoType = psoType; }
	PSOType GetPSOType() { return m_psoType; }

	shared_ptr<Texture> GetDefaultTexture() { return m_defaultTex; }

	void CommintGlobalData();
private:
	void UpdateGlobalConstants(const float& dt, const Vector3& eyeWorld,
		const Matrix& viewRow,
		const Matrix& projRow, const Matrix& refl);

	//void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();
	virtual void OnMouseMove(int mouseX, int mouseY);
	virtual void OnMouseClick(int mouseX, int mouseY);
private:
	// �׷��Ƚ� ����
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
	shared_ptr<GraphicsPSO> m_skinnedGraphicsPSO;
	shared_ptr<GraphicsPSO> m_skyboxGraphicsPSO;
	shared_ptr<GraphicsPSO> m_postEffectGraphicsPSO;
	shared_ptr<GraphicsPSO> m_shadowGraphicsPSO;
	shared_ptr<GraphicsPSO> m_shadowSkinnedGraphicsPSO;

	UINT m_numQualityLevels = 0;

	PSOType m_psoType;
public:
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> m_rtGroups;

	shared_ptr<ConstantBuffer<GlobalConstants>> m_globalConstsBuffer;
	shared_ptr<ConstantBuffer<GlobalConstants>> m_shadowGlobalConstsBuffer[MAX_LIGHTS_COUNT];

	shared_ptr<Texture> m_envTex;
	shared_ptr<Texture> m_irradianceTex;
	shared_ptr<Texture> m_specularTex;
	shared_ptr<Texture> m_brdfTex;
	// TODO. �ؽ�ó �ӽ� ����, ���� �߻� ��������..
	// �� �� ���� ��� ã�ƺ���..��
	shared_ptr<Texture> m_defaultTex;

	// ������ ����
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

	// ������ ����
	hlab::Camera m_camera;
	// DaerimGTA
	bool m_lightRotate = false;

	shared_ptr<DModel> m_skybox;
	shared_ptr<DModel> m_screenSquare;
	shared_ptr<DModel> m_ground;
	shared_ptr<Wizard> m_wizard;
	// EDaerimGTA
};
}


