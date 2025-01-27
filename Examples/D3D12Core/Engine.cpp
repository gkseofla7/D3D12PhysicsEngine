#include "Engine.h"
#include "Device.h"
#include "ConstantBuffer.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "RootSignature.h"
#include "Shader.h"
#include "GraphicsPipelineState.h"
#include "GraphicsPSO2.h"
#include "Samplers2.h"
#include "ConstantBuffer.h"
#include "../GameCore/DSkinnedMeshModel2.h"
#include "../GameCore/MeshLoadHelper2.h"
#include "../GameCore/GeometryGenerator2.h"
#include "../GameCore/Wizard2.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);
namespace dengine {


void ResolveMSAATexture(
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* msaaRenderTarget,
    ID3D12Resource* resolvedTarget,
    DXGI_FORMAT format);

bool CheckMultisampleQualityLevels(ComPtr<ID3D12Device> device, DXGI_FORMAT format, UINT sampleCount)
{
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels = {};
    msQualityLevels.Format = format;
    msQualityLevels.SampleCount = sampleCount;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

    HRESULT hr = device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)
    );

    if (SUCCEEDED(hr)) {
        printf("SampleCount: %u, NumQualityLevels: %u\n",
               msQualityLevels.SampleCount,
               msQualityLevels.NumQualityLevels);
		return true;
    } else {
        printf("Failed to check multisample quality levels.\n");
		return false;
    }
}

LRESULT WINAPI WndProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return GEngine->MsgProc(hWnd, msg, wParam, lParam);
}
Engine::~Engine()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(m_window.hwnd);
}
void Engine::Init(const WindowInfo& info)
{
	m_window = info;

	InitMainWindow();
	InitGraphics();
	InitGUI();
	InitScene();
}
int Engine::Run()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{ 
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
			ImGui_ImplWin32_NewFrame(); 
			ImGui_ImplDX12_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Scene Control");

			// ImGui가 측정해주는 Framerate 출력
			ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

			//UpdateGUI(); // 추가적으로 사용할 GUI

			ImGui::End();
			ImGui::Render();
			Update(ImGui::GetIO().DeltaTime);

			Render();

		}
	}
	_ASSERT(_CrtCheckMemory());

	return 0;
}

bool Engine::InitMainWindow()
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX),
					 CS_CLASSDC,
					 WndProc2,
					 0L,
					 0L,
					 GetModuleHandle(NULL),
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 L"Daerim'sGTA", // lpszClassName, L-string
					 NULL };

	if (!RegisterClassEx(&wc)) {
		std::cout << "RegisterClassEx() failed." << std::endl;
		return false;
	}

	RECT wr = { 0, 0, m_window.width, m_window.height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
	m_window.hwnd = CreateWindow(wc.lpszClassName, L"Daerim'sGTA",
		WS_OVERLAPPEDWINDOW,
		100, // 윈도우 좌측 상단의 x 좌표
		100, // 윈도우 좌측 상단의 y 좌표
		wr.right - wr.left, // 윈도우 가로 방향 해상도
		wr.bottom - wr.top, // 윈도우 세로 방향 해상도
		NULL, NULL, wc.hInstance, NULL);

	if (!m_window.hwnd) {
		std::cout << "CreateWindow() failed." << std::endl;
		return false;
	}

	ShowWindow(m_window.hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_window.hwnd);

	SetForegroundWindow(m_window.hwnd);
	return true;
}

bool Engine::InitGUI() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.DisplaySize = ImVec2(float(m_window.width), float(m_window.height));
	ImGui::StyleColorsLight();

	if (!ImGui_ImplWin32_Init(m_window.hwnd))
	{
		return false;
	}

	// Setup Platform/Renderer backends
	if (!ImGui_ImplDX12_Init(DEVICE.Get(), SWAP_CHAIN_BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM,
		GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetShaderResourceHeap().Get(),
		GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetShaderResourceHeap()->GetCPUDescriptorHandleForHeapStart(),
		GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetShaderResourceHeap()->GetGPUDescriptorHandleForHeapStart()))
	{
		return false;
	}

	return true;
}

void Engine::InitGraphics()
{
	// 그려질 화면 크기를 설정
	m_viewport = { 0, 0, static_cast<FLOAT>(m_window.width), static_cast<FLOAT>(m_window.height), 0.0f, 1.0f };
	m_scissorRect = CD3DX12_RECT(0, 0, m_window.width, m_window.height);

	m_device = std::make_shared<Device>();
	m_device->Init();

	m_graphicsCmdQueue = std::make_shared<GraphicsCommandQueue>();
	m_graphicsCmdQueue->Init(m_device->GetDevice());

	m_swapChain = std::make_shared< SwapChain>();
	m_swapChain->Init(m_window, m_device->GetDevice(), m_device->GetDXGI(), m_graphicsCmdQueue->GetCmdQueue());

	m_graphicsCmdQueue->SetSwapChain(m_swapChain);

	m_graphicsDescHeap = std::make_shared< GraphicsDescriptorHeap>();
	m_graphicsDescHeap->Init(256);

	m_shader = std::make_shared<Shader>();
	m_shader->Init();

	m_samplers = std::make_shared<Samplers>();
	m_samplers->Init();

	m_rootSignature = std::make_shared<RootSignature>();
	m_rootSignature->Init();

	m_graphicsPipelineState = std::make_shared< GraphicsPipelineState>();
	m_graphicsPipelineState->Init();

	InitPSO();
	InitGlobalBuffer();
}

void Engine::InitPSO()
{
	m_defaultGraphicsPSO = std::make_shared< GraphicsPSO>();
	m_defaultGraphicsPSO->Init(m_rootSignature->GetGraphicsRootSignature(), m_graphicsPipelineState->GetDefaultPipelineState(), PSOType::DEFAULT);

	m_skinnedGraphicsPSO = std::make_shared< GraphicsPSO>();
	m_skinnedGraphicsPSO->Init(m_rootSignature->GetGraphicsRootSignature(), m_graphicsPipelineState->GetSkinnedPipelineState(), PSOType::DEFAULT);

	m_skyboxGraphicsPSO = std::make_shared< GraphicsPSO>();
	m_skyboxGraphicsPSO->Init(m_rootSignature->GetSkyboxRootSignature(), m_graphicsPipelineState->GetSkyboxPipelineState(), PSOType::SKYBOX);

	m_postEffectGraphicsPSO = std::make_shared<GraphicsPSO>();
	m_postEffectGraphicsPSO->Init(m_rootSignature->GetSamplingRootSignature(), m_graphicsPipelineState->GetPostEffectPipelineState(), PSOType::SAMPLING);

	m_shadowGraphicsPSO = std::make_shared<GraphicsPSO>();
	m_shadowGraphicsPSO->Init(m_rootSignature->GetGraphicsRootSignature(), m_graphicsPipelineState->GetShadowPipelineState(), PSOType::SHADOW);

	m_shadowSkinnedGraphicsPSO = std::make_shared<GraphicsPSO>();
	m_shadowSkinnedGraphicsPSO->Init(m_rootSignature->GetGraphicsRootSignature(), m_graphicsPipelineState->GetShadowSkinnedPipelineState(), PSOType::SHADOW);
}

void Engine::InitGlobalBuffer()
{
	m_globalConstsBuffer = std::make_shared<ConstantBuffer<GlobalConstants>>();
	m_globalConstsBuffer->Init(CBV_REGISTER::b0, SWAP_CHAIN_BUFFER_COUNT);

	for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
	{
		m_shadowGlobalConstsBuffer[i] = std::make_shared<ConstantBuffer<GlobalConstants>>();
		m_shadowGlobalConstsBuffer[i]->Init(CBV_REGISTER::b0, SWAP_CHAIN_BUFFER_COUNT);
	}

	m_envTex = std::make_shared<Texture>();
	m_irradianceTex = std::make_shared<Texture>();
	m_specularTex = std::make_shared<Texture>();
	m_brdfTex = std::make_shared<Texture>();

	ComPtr<ID3D12Resource> backBuffer;
	m_swapChain->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();
	m_emptyTex = make_shared<Texture>();
	m_emptyTex->Create(desc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_NONE);

	CreateRenderTargetGroups();
}

void Engine::InitCubemaps(wstring basePath, wstring envFilename,
	wstring specularFilename, wstring irradianceFilename,
	wstring brdfFilename) 
{
	m_envTex->RegisterOnLoadCallback([this]() {
		this->GetGraphicsDescHeap()->SetGlobalSRVForAllFrame(m_envTex->GetSRVHandle(), SRV_REGISTER::t10);
		});
	m_envTex->LoadTexture((basePath + envFilename).c_str(), true);

	m_irradianceTex->RegisterOnLoadCallback([this]() {
		this->GetGraphicsDescHeap()->SetGlobalSRVForAllFrame(m_irradianceTex->GetSRVHandle(), SRV_REGISTER::t11);
		});
	m_irradianceTex->LoadTexture((basePath + irradianceFilename).c_str(), true);

	m_specularTex->RegisterOnLoadCallback([this]() {
		this->GetGraphicsDescHeap()->SetGlobalSRVForAllFrame(m_specularTex->GetSRVHandle(), SRV_REGISTER::t12);
		});
	m_specularTex->LoadTexture((basePath + specularFilename).c_str(), true);

	// BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
	m_brdfTex->RegisterOnLoadCallback([this]() {
		this->GetGraphicsDescHeap()->SetGlobalSRVForAllFrame(m_brdfTex->GetSRVHandle(), SRV_REGISTER::t13);
		});
	m_brdfTex->LoadTexture((basePath + brdfFilename).c_str(), false);
}

bool Engine::InitScene()
{
	// 조명 설정
	{
		GlobalConstants& globalConstsCPU = m_globalConstsBuffer->GetCpu();
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
		// DaerimGTA
		globalConstsCPU.strengthIBL = 0.1f;
		globalConstsCPU.lodBias = 0.0f;
	}

	m_camera.Reset(Vector3(1.60851f, 0.409084f, 0.560064f), -1.65915f,
		0.0654498f);

	InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
		L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
		L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

	{
		std::string path = "../Assets/Characters/Mixamo/";
		std::string characterName = "character.fbx";
		Vector3 center(0.5f, 0.1f, 1.0f);
		shared_ptr<DSkinnedMeshModel> wizardModel = std::make_shared<DSkinnedMeshModel>(path, characterName);
		MaterialConstants materialConsts;
		materialConsts.albedoFactor = Vector3(1.0f);
		materialConsts.roughnessFactor = 0.8f;
		materialConsts.metallicFactor = 0.0f;
		wizardModel->SetMaterialConstants(materialConsts);
		wizardModel->UpdateWorldRow(Matrix::CreateScale(0.2f) *
			Matrix::CreateTranslation(center));
		wizardModel->SetScale(0.2f);

		m_wizard = make_shared<Wizard>(wizardModel);
		m_wizard->Initialize(wizardModel);

		m_activateActor = m_wizard;
	}
	{
		string meshKey = MeshLoadHelper::LoadBoxMesh(40.0f, true);
		m_skybox = std::make_shared<DModel>(meshKey);
		m_modelList.push_back(m_skybox);
	}
	{
		auto mesh = GeometryGenerator::MakeSquare(5.0, { 10.0f, 10.0f });
		string path = "../Assets/Textures/PBR/stringy-marble-ue/";
		mesh.albedoTextureFilename = path + "stringy_marble_albedo.png";
		mesh.emissiveTextureFilename = "";
		mesh.aoTextureFilename = path + "stringy_marble_ao.png";
		mesh.metallicTextureFilename = path + "stringy_marble_Metallic.png";
		mesh.normalTextureFilename = path + "stringy_marble_Normal-dx.png";
		mesh.roughnessTextureFilename = path + "stringy_marble_Roughness.png";
		std::string meshKey = "Ground";
		vector<MeshData> meshDataList;
		meshDataList.push_back(mesh);
		MeshLoadHelper::LoadModel(meshKey, meshDataList);

		m_ground = make_shared<DModel>(meshKey);
		MaterialConstants materialConsts;
		materialConsts.albedoFactor = Vector3(0.2f);
		materialConsts.emissionFactor = Vector3(0.2f);
		materialConsts.roughnessFactor = 0.3f;
		materialConsts.metallicFactor = 0.5f;
		m_ground->SetMaterialConstants(materialConsts);

		Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
		m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
			Matrix::CreateTranslation(position));
	}
	{
		string meshKey = MeshLoadHelper::LoadSquareMesh();
		m_screenSquare = make_shared<DModel>(meshKey);
	}

	m_camera.SetAspectRatio(this->GetAspectRatio());

	return true;
}

void Engine::Update(float dt)
{
	MeshLoadHelper::LoadAllUnloadedModel();

	GetGraphicsCmdQueue()->WaitFrameSync(BACKBUFFER_INDEX);

	m_camera.UpdateKeyboard(dt, m_keyPressed);

	m_wizard->Tick(dt);
	m_skybox->Tick(dt);
	m_screenSquare->Tick(dt);
	m_ground->Tick(dt);

	UpdateGlobalConstants(dt);
}

void Engine::Render()
{
	RenderBegin();
	
	RenderShadowMaps();

	RenderOpaqueObjects();

	PostRender();
	RenderEnd(); 
}

void Engine::RenderOpaqueObjects()
{
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->OMSetRenderTargets(1, 0);

	m_skyboxGraphicsPSO->UploadGraphicsPSO();
	m_skybox->Render();

	m_skinnedGraphicsPSO->UploadGraphicsPSO();
	m_wizard->Render();

	m_defaultGraphicsPSO->UploadGraphicsPSO();
	m_ground->Render();
}

void Engine::RenderShadowMaps()
{
	GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->WaitResourceToTarget();
	const GlobalConstants& globalConstsCPU = m_globalConstsBuffer->GetCpu();
	for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
	{
		if (globalConstsCPU.lights[i].type & LIGHT_SHADOW)
		{
			GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->ClearRenderTargetView(i);
			GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->OMSetRenderTargets(1, i);

			// Render Skinned
			m_shadowSkinnedGraphicsPSO->UploadGraphicsPSO();
			GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(2, m_shadowGlobalConstsBuffer[i]->GetGpuVirtualAddress(0));
			m_wizard->Render();
			// Render Default
			m_shadowGraphicsPSO->UploadGraphicsPSO();
			GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(2, m_shadowGlobalConstsBuffer[i]->GetGpuVirtualAddress(0));
			m_ground->Render();
		}
	}
	GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->WaitTargetToResource();
	GetGraphicsDescHeap()->SetGlobalSRV(GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->GetShaderResourceHeap()->GetCPUDescriptorHandleForHeapStart(), SRV_REGISTER::t15, MAX_LIGHTS_COUNT);
}

void Engine::PostRender() 
{
	m_postEffectGraphicsPSO->UploadGraphicsPSO();
	int8 backIndex = m_swapChain->GetBackBufferIndex();
	ResolveMSAATexture(GRAPHICS_CMD_LIST.Get(), GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->GetRTTexture(0)->GetTex2D().Get()
		, GetRTGroup(RENDER_TARGET_GROUP_TYPE::RESOLVE)->GetRTTexture(0)->GetTex2D().Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);

	GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->ClearRenderTargetView(0);
	GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->OMSetRenderTargets(1, 0);

	
	GetGraphicsDescHeap()->SetSRV(GetRTGroup(RENDER_TARGET_GROUP_TYPE::RESOLVE)->GetRTTexture(0)->GetSRVHandle()
		, SRV_REGISTER::t0);
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(0, m_samplers->GetDescHeap()->GetGPUDescriptorHandleForHeapStart());
	GEngine->GetGraphicsDescHeap()->CommitTableForSampling();

	m_screenSquare->Render();
}

void Engine::RenderBegin()
{
	m_graphicsCmdQueue->RenderBegin();
}

void Engine::RenderEnd()
{
	SetMainViewport();
	m_graphicsCmdQueue->RenderEnd();
}
 
void Engine::SetMainViewport()
{
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = m_window.width;
	viewport.Height = m_window.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	GRAPHICS_CMD_LIST->RSSetViewports(1, &viewport);
}

void Engine::ResizeWindow(int32 width, int32 height)
{
	m_window.width = width;
	m_window.height = height;

	RECT rect = { 0, 0, width, height };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	::SetWindowPos(m_window.hwnd, 0, 100, 100, width, height, 0);
}

// 여러 물체들이 공통적으료 사용하는 Const 업데이트
void Engine::SetGlobalConstantsCpu(const float& dt, const Vector3& eyeWorld,
	const Matrix& viewRow,
	const Matrix& projRow, const Matrix& refl) 
{
	GlobalConstants& globalCpuData = m_globalConstsBuffer->GetCpu();
	globalCpuData.globalTime += dt;
	globalCpuData.eyeWorld = eyeWorld;
	globalCpuData.view = viewRow.Transpose();
	globalCpuData.proj = projRow.Transpose();
	globalCpuData.invProj = projRow.Invert().Transpose();
	globalCpuData.viewProj = (viewRow * projRow).Transpose();
	globalCpuData.invView = viewRow.Invert().Transpose();

	// 그림자 렌더링에 사용
	globalCpuData.invViewProj = globalCpuData.viewProj.Invert();
}

void Engine::UpdateLightsCpu(float dt)
{
	// 회전하는 lights[1] 업데이트
	static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
	if (m_lightRotate)
	{
		lightDev = Vector3::Transform(lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
	}

	GlobalConstants& globalConstsCPU = m_globalConstsBuffer->GetCpu();
	globalConstsCPU.lights[1].position = Vector3(0.0f, 1.1f, 2.0f) + lightDev;
	Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
	globalConstsCPU.lights[1].direction =
		focusPosition - globalConstsCPU.lights[1].position;
	globalConstsCPU.lights[1].direction.Normalize();

	for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
	{
		const auto& light = m_globalConstsBuffer->GetCpu().lights[i];
		if (light.type & LIGHT_SHADOW) {

			Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
			if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
				up = Vector3(1.0f, 0.0f, 0.0f);

			// 그림자맵을 만들 때 필요
			Matrix lightViewRow = XMMatrixLookAtLH(
				light.position, light.position + light.direction, up);

			Matrix lightProjRow = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

			GlobalConstants& shadowGlobalConstsCPU = m_shadowGlobalConstsBuffer[i]->GetCpu();
			shadowGlobalConstsCPU.eyeWorld = light.position;
			shadowGlobalConstsCPU.view = lightViewRow.Transpose();
			shadowGlobalConstsCPU.proj = lightProjRow.Transpose();
			shadowGlobalConstsCPU.invProj =
				lightProjRow.Invert().Transpose();
			shadowGlobalConstsCPU.viewProj =
				(lightViewRow * lightProjRow).Transpose();

			// LIGHT_FRUSTUM_WIDTH 확인
			// Vector4 eye(0.0f, 0.0f, 0.0f, 1.0f);
			// Vector4 xLeft(-1.0f, -1.0f, 0.0f, 1.0f);
			// Vector4 xRight(1.0f, 1.0f, 0.0f, 1.0f);
			// eye = Vector4::Transform(eye, lightProjRow);
			// xLeft = Vector4::Transform(xLeft, lightProjRow.Invert());
			// xRight = Vector4::Transform(xRight, lightProjRow.Invert());
			// xLeft /= xLeft.w;
			// xRight /= xRight.w;
			// cout << "LIGHT_FRUSTUM_WIDTH = " << xRight.x - xLeft.x <<
			// endl;

			// 그림자를 실제로 렌더링할 때 필요
			globalConstsCPU.lights[i].viewProj =
				shadowGlobalConstsCPU.viewProj;
			globalConstsCPU.lights[i].invProj =
				shadowGlobalConstsCPU.invProj;

			// 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서 넣어줌

		}
	}
}

void Engine::UpdateGlobalConstants(float dt)
{
	// 반사 행렬 추가
	const Vector3 eyeWorld = m_camera.GetEyePos();
	const Matrix reflectRow; //Matrix::CreateReflection(m_mirrorPlane);
	const Matrix viewRow = m_camera.GetViewRow();
	const Matrix projRow = m_camera.GetProjRow();
	SetGlobalConstantsCpu(dt, eyeWorld, viewRow, projRow, reflectRow);
	UpdateLightsCpu(dt);

	m_globalConstsBuffer->Upload();
	for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
	{
		m_shadowGlobalConstsBuffer[i]->Upload();
	}
}

void Engine::CommitGlobalData()
{
	// b0
	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(2, m_globalConstsBuffer->GetGpuVirtualAddress(0));
	// s0~s6
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(0, m_samplers->GetDescHeap()->GetGPUDescriptorHandleForHeapStart());
	GetGraphicsDescHeap()->CommitGlobalTable();
}

void Engine::CreateRenderTargetGroups()
{
	// SwapChain Group
	{
		for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			RenderTarget renderTarget;
			ComPtr<ID3D12Resource> resource;
			m_swapChain->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&resource));
			renderTarget.target = std::make_shared<Texture>();
			renderTarget.target->CreateFromResource(resource);
			for (int j = 0; j < 4; j++) {
				renderTarget.clearColor[j] = 0.0f;
			}
			vector<RenderTarget> rtVec;
			rtVec.push_back(renderTarget);
			resource->SetName(L"SwapChainTexture"); 

			shared_ptr<Texture> dsTexture = std::make_shared<Texture>();
			CD3DX12_RESOURCE_DESC rscDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_window.width, m_window.height);
			dsTexture->Create(rscDesc,CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

			vector< shared_ptr<Texture>> dsTextures;
			dsTextures.push_back(dsTexture);

			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)] = std::make_shared<RenderTargetGroup>();
			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTextures);
		}
	}

	if (CheckMultisampleQualityLevels(DEVICE, DXGI_FORMAT_R16G16B16A16_FLOAT, 4))
	{
		m_numQualityLevels = 1;
	}
	D3D12_RESOURCE_DESC backBufferDepthDesc = GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetDSTexture(0)->GetTex2D()->GetDesc();
	{
		// DepthStencilView
		D3D12_RESOURCE_DESC depthDesc = backBufferDepthDesc;
		if (m_numQualityLevels)
		{
			depthDesc.SampleDesc.Count = 4;
			depthDesc.SampleDesc.Quality = m_numQualityLevels - 1;
		}
		else
		{
			depthDesc.SampleDesc.Count = 1;
			depthDesc.SampleDesc.Quality = 0;
		}
		depthDesc.MipLevels = 1;

		D3D12_RESOURCE_DESC desc;
		// FLOAT MSAA
		ComPtr<ID3D12Resource> backBuffer;
		m_swapChain->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		desc = backBuffer->GetDesc();

		desc.MipLevels = desc.DepthOrArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;


		if (m_numQualityLevels)
		{
			desc.SampleDesc.Count = 4;
			desc.SampleDesc.Quality = m_numQualityLevels - 1;
		}
		else
		{
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
		}

		for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			RenderTarget renderTarget;
			renderTarget.target = std::make_shared<Texture>();
			renderTarget.target->Create(desc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
			renderTarget.target->GetTex2D()->SetName(L"FloatBufferTexture");
			for (int j = 0; j < 4; j++) {
				renderTarget.clearColor[j] = 0.0f;
			}
			vector<RenderTarget> rtVec;
			rtVec.push_back(renderTarget);

			shared_ptr<Texture> dsMultiSamplingTexture = std::make_shared<Texture>();
			dsMultiSamplingTexture->Create(depthDesc,
				CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			vector <shared_ptr<Texture>> dsTextures;
			dsTextures.push_back(dsMultiSamplingTexture);

			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::FLOAT)] = std::make_shared<RenderTargetGroup>();
			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::FLOAT)]->Create(RENDER_TARGET_GROUP_TYPE::FLOAT, rtVec, dsTextures);
		}

		// FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;


		for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			RenderTarget renderTarget;
			renderTarget.target = std::make_shared<Texture>();
			renderTarget.target->Create(desc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
			renderTarget.target->GetTex2D()->SetName(L"ResolveTexture");
			vector<RenderTarget> resolveRtVec;
			resolveRtVec.push_back(renderTarget);

			shared_ptr<Texture> dsMultiSamplingTexture = std::make_shared<Texture>();
			dsMultiSamplingTexture->Create(depthDesc,
				CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			vector <shared_ptr<Texture>> dsTextures;
			dsTextures.push_back(dsMultiSamplingTexture);

			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::RESOLVE)] = std::make_shared<RenderTargetGroup>();
			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::RESOLVE)]->Create(RENDER_TARGET_GROUP_TYPE::RESOLVE, resolveRtVec, dsTextures);
		}
	}
	 
	// Shadow
	{
		for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
		{
			D3D12_RESOURCE_DESC depthDesc = backBufferDepthDesc;

			vector<RenderTarget> rtVec(MAX_LIGHTS_COUNT);
			for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
			{
				rtVec[i].target = std::make_shared<Texture>();
				CD3DX12_RESOURCE_DESC rscDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 1280, 1280);
				rtVec[i].target->Create(rscDesc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
					Vector4(1.0f, 1.0f, 1.0f, 1.0f));
				for (int j = 0; j < 4; j++) {
					rtVec[i].clearColor[j] = 1.0f;
				}
			}

			vector <shared_ptr<Texture>> dsTextures;
			shared_ptr<Texture> dsTexture = std::make_shared<Texture>();
			depthDesc.Width = 1280;
			depthDesc.Height = 1280;
			dsTexture->Create(depthDesc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			dsTextures.push_back(dsTexture);

			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)] = make_shared<RenderTargetGroup>();
			m_rtGroups[i][static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)]->Create(RENDER_TARGET_GROUP_TYPE::SHADOW, rtVec, dsTextures);
		}
	}
}

void Engine::OnMouseMove(int mouseX, int mouseY) {

	m_mouseX = mouseX;
	m_mouseY = mouseY;

	// 마우스 커서의 위치를 NDC로 변환
	// 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
	// NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
	m_mouseNdcX = mouseX * 2.0f / m_window.width - 1.0f;
	m_mouseNdcY = -mouseY * 2.0f / m_window.height + 1.0f;

	// 커서가 화면 밖으로 나갔을 경우 범위 조절
	// 게임에서는 클램프를 안할 수도 있습니다.
	m_mouseNdcX = std::clamp(m_mouseNdcX, -1.0f, 1.0f);
	m_mouseNdcY = std::clamp(m_mouseNdcY, -1.0f, 1.0f);

	// 카메라 시점 회전
	m_camera.UpdateMouse(m_mouseNdcX, m_mouseNdcY);
}

void Engine::OnMouseClick(int mouseX, int mouseY) {

	m_mouseX = mouseX;
	m_mouseY = mouseY;

	m_mouseNdcX = mouseX * 2.0f / m_window.windowed - 1.0f;
	m_mouseNdcY = -mouseY * 2.0f / m_window.height + 1.0f;
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		// 화면 해상도가 바뀌면 SwapChain을 다시 생성
		if (m_swapChain) {

			//m_window.width = int(LOWORD(lParam));
			//m_window.height = int(HIWORD(lParam));

			//// 윈도우가 Minimize 모드에서는 screenWidth/Height가 0
			//if (m_window.width && m_window.height) {

			//	std::cout << "Resize SwapChain to " << m_window.width << " "
			//		<< m_window.height << std::endl;

			//	// ViewPort 관련 Reset 
			//	// TODO. 제대로 날리고 있는지 확인 필요
			//	m_swapChain->Init(m_window, DEVICE, m_device->GetDXGI(), m_graphicsCmdQueue->GetCmdQueue());
			//	vector<RenderTarget> rtVec(SWAP_CHAIN_BUFFER_COUNT);
			//	for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
			//	{
			//		ComPtr<ID3D12Resource> resource;
			//		m_swapChain->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&resource));
			//		rtVec[i].target = std::make_shared<Texture>();
			//		rtVec[i].target->CreateFromResource(resource);
			//	}
			//	// DepthStencil
			//	shared_ptr<Texture> dsTexture = std::make_shared<Texture>();
			//	dsTexture->Create(DXGI_FORMAT_D32_FLOAT, m_window.width, m_window.height,
			//		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			//		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			//	m_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTexture);
			//	//SetMainViewport();


			//	m_camera.SetAspectRatio(this->GetAspectRatio());
			//}
		}
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		if (!m_leftButton) {
			m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
		}
		m_leftButton = true;
		OnMouseClick(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		m_leftButton = false;
		break;
	case WM_RBUTTONDOWN:
		if (!m_rightButton) {
			m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
		}
		m_rightButton = true;
		break;
	case WM_RBUTTONUP:
		m_rightButton = false;
		break;
	case WM_KEYDOWN:
		if (m_activateActor != nullptr)
		{
			if (m_activateActor->MsgProc(wParam, true))
			{
				return true;
			}
		}
		m_keyPressed[wParam] = true;
		//if (wParam == VK_ESCAPE) { // ESC키 종료
		//	DestroyWindow(hwnd);
		//}
		//if (wParam == VK_SPACE) {
		//	m_lightRotate = !m_lightRotate;
		//}
		break;
	case WM_KEYUP:
		if (m_activateActor != nullptr)
		{
			if (m_activateActor->MsgProc(wParam, false))
			{
				return true;
			}
		}
		if (wParam == 'F') { // f키 일인칭 시점
			m_camera.m_useFirstPersonView = !m_camera.m_useFirstPersonView;
		}

		m_keyPressed[wParam] = false;
		break;
	case WM_MOUSEWHEEL:
		m_wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

shared_ptr<RenderTargetGroup> Engine::GetRTGroup(RENDER_TARGET_GROUP_TYPE type)
{
	return m_rtGroups[BACKBUFFER_INDEX][static_cast<uint8>(type)]; 
}

float Engine::GetAspectRatio() const
{
	return float(m_window.width) / m_window.height;
}


void ResolveMSAATexture(
	ID3D12GraphicsCommandList* commandList,
	ID3D12Resource* msaaRenderTarget,
	ID3D12Resource* resolvedTarget,
	DXGI_FORMAT format)
{
	// Ensure the resources are in the correct states
	D3D12_RESOURCE_BARRIER barriers[2] = {};

	// Transition MSAA render target to RESOLVE_SOURCE state
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Transition.pResource = msaaRenderTarget;
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// Transition resolved target to RESOLVE_DEST state
	barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[1].Transition.pResource = resolvedTarget;
	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON; // Or appropriate state
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST;
	barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// Apply the barriers
	commandList->ResourceBarrier(2, barriers);

	// Resolve the MSAA render target into the resolved target
	commandList->ResolveSubresource(
		resolvedTarget,  // Destination resource
		0,               // Destination subresource index
		msaaRenderTarget,// Source resource
		0,               // Source subresource index
		format           // Format (must match the resource format)
	);

	// Transition resources back to their original states if needed
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

	barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
	barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

	commandList->ResourceBarrier(2, barriers);
}
}
