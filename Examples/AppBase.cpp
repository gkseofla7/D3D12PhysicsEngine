#include "AppBase.h"

#include <algorithm>
#include <directxtk/SimpleMath.h>

#include "D3D11Utils.h"
#include "GraphicsCommon.h"
#include "Actor.h"
#include "DaerimsEngineBase.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

AppBase *g_appBase = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_screenWidth(1280), m_screenHeight(720), m_mainWindow(0),
      m_screenViewport(D3D11_VIEWPORT()) {

    g_appBase = this;

    m_camera.SetAspectRatio(this->GetAspectRatio());
}

AppBase::~AppBase() {
    g_appBase = nullptr;

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_mainWindow);
    // UnregisterClass(wc.lpszClassName, wc.hInstance);//생략

    // COMPtr에서 알아서 release
    // ComPtr automatically maintains a reference count for the underlying
    // interface pointer and releases the interface when the reference count
    // goes to zero.
    // https:learn.microsoft.com/en-us/cpp/cppcx/wrl/comptr-class?view=msvc-170
    // 예시: m_d3dDevice.Reset(); 생략
}

float AppBase::GetAspectRatio() const {
    return float(m_screenWidth) / m_screenHeight;
}

int AppBase::Run() {

    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            // ImGui가 측정해주는 Framerate 출력
            ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            UpdateGUI(); // 추가적으로 사용할 GUI

            ImGui::End();
            ImGui::Render();
            Update(ImGui::GetIO().DeltaTime);

            ThreadPool& threadPool = ThreadPool::getInstance();

            // 렌더 스레드 작업 완료를 기다림
            {
                std::unique_lock<std::mutex> lock(threadPool.m_render_job_q_);
                threadPool.cv_render_job_q_.wait(lock, [&]() { return threadPool.IsRenderThreadDone(); });
                threadPool.SetUsingMainThreadUsingRendering(true);
            }
            Render(); 


            // Example의 Render()에서 RT 설정을 해주지 않았을 경우에도
            // 백 버퍼에 GUI를 그리기위해 RT 설정
            // 예) Render()에서 ComputeShader만 사용
            AppBase::SetMainViewport();
            m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(),
                                          NULL);

            // GUI 렌더링
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // GUI 렌더링 후에 Present() 호출
            m_swapChain->Present(1, 0);

            threadPool.SetUsingMainThreadUsingRendering(false);
            threadPool.cv_render_job_q_.notify_one();
        }
    }

    return 0;
}

bool AppBase::Initialize() {

    if (!InitMainWindow())
        return false;
    ThreadPool& tPool = ThreadPool::getInstance();
    {
        std::unique_lock<std::mutex> lock(tPool.m_render_job_q_);
        tPool.SetUsingMainThreadUsingRendering(true);
    }
    bool bInitDirect = InitDirect3D();
    tPool.SetUsingMainThreadUsingRendering(false);
    tPool.cv_render_job_q_.notify_one();

    if (!bInitDirect)
        return false;

    if (!InitGUI())
        return false;

    if (!InitScene())
        return false;

    // PostEffect에 사용
    m_screenSquare = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeSquare()});

    // 환경 박스 초기화
    MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
    std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
    m_skybox = make_shared<Model>(m_device, m_context, vector{skyboxMesh});
    m_skybox->m_name = "SkyBox";

    // 콘솔창이 렌더링 창을 덮는 것을 방지
    SetForegroundWindow(m_mainWindow);

    return true;
}

// 여러 예제들이 공통적으로 사용하기 좋은 장면 설정
bool AppBase::InitScene() {

    // 조명 설정
    {
        // 조명 0은 고정
        m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[0].position = Vector3(0.0f, 1.5f, 1.1f);
        m_globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
        m_globalConstsCPU.lights[0].spotPower = 3.0f;
        m_globalConstsCPU.lights[0].radius = 0.04f;
        m_globalConstsCPU.lights[0].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 1의 위치와 방향은 Update()에서 설정
        m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[1].spotPower = 3.0f;
        m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
        m_globalConstsCPU.lights[1].radius = 0.02f;
        m_globalConstsCPU.lights[1].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 2는 꺼놓음
        m_globalConstsCPU.lights[2].type = LIGHT_OFF;
    }

    // TODO. 모두 Object로 변환 필요
    return true;
    // 조명 위치 표시
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            m_lightSphere[i] =
                make_shared<Model>(m_device, m_context, vector{sphere});
            m_lightSphere[i]->UpdateWorldRow(Matrix::CreateTranslation(
                m_globalConstsCPU.lights[i].position));
            m_lightSphere[i]->m_materialConsts.GetCpu().albedoFactor =
                Vector3(0.0f);
            m_lightSphere[i]->m_materialConsts.GetCpu().emissionFactor =
                Vector3(1.0f, 1.0f, 0.0f);
            m_lightSphere[i]->m_castShadow =
                false; // 조명 표시 물체들은 그림자 X

            // if (m_globalConstsCPU.lights[i].type == 0)
            m_lightSphere[i]->m_isVisible = false;
            m_lightSphere[i]->m_name = "LightSphere" + std::to_string(i);
            m_lightSphere[i]->m_isPickable = false;

            m_basicList.push_back(m_lightSphere[i]); // 리스트에 등록
        }
    }

    // 커서 표시 (Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구)
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        m_cursorSphere =
            make_shared<Model>(m_device, m_context, vector{sphere});
        m_cursorSphere->m_isVisible = false; // 마우스가 눌렸을 때만 보임
        m_cursorSphere->m_castShadow = false; // 그림자 X
        m_cursorSphere->m_materialConsts.GetCpu().albedoFactor = Vector3(0.0f);
        m_cursorSphere->m_materialConsts.GetCpu().emissionFactor =
            Vector3(0.0f, 1.0f, 0.0f);

        m_basicList.push_back(m_cursorSphere); // 리스트에 등록
    }

    return true;
}

void AppBase::Update(float dt) {

    KillObjects();
    // 카메라의 이동
    m_camera.UpdateKeyboard(dt, m_keyPressed);

    // 반사 행렬 추가
    const Vector3 eyeWorld = m_camera.GetEyePos();
    const Matrix reflectRow = Matrix::CreateReflection(m_mirrorPlane);
    const Matrix viewRow = m_camera.GetViewRow();
    const Matrix projRow = m_camera.GetProjRow();

    UpdateLights(dt);

    // 공용 ConstantBuffer 업데이트
    AppBase::UpdateGlobalConstants(dt, eyeWorld, viewRow, projRow, reflectRow);

    // 거울은 따로 처리
    if (m_mirror)
        m_mirror->UpdateConstantBuffers(m_device, m_context);

    // 조명의 위치 반영
    //for (int i = 0; i < MAX_LIGHTS; i++)
    //    m_lightSphere[i]->UpdateWorldRow(
    //        Matrix::CreateScale(
    //            std::max(0.01f, m_globalConstsCPU.lights[i].radius)) *
    //        Matrix::CreateTranslation(m_globalConstsCPU.lights[i].position));

    ProcessMouseControl();
    for (auto &i : m_basicList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }

    for (int i  = 0; i<m_objectList.size(); i++)
    {
        m_objectList[i]->Tick(dt);
        // TODO. Tick으로 빼는게 좋을듯 싶은데,,
        m_objectList[i]->GetModel()->UpdateConstantBuffers(m_context);
    }
}

void AppBase::UpdateLights(float dt) {

    // 회전하는 lights[1] 업데이트
    static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
    if (m_lightRotate) {
        lightDev = Vector3::Transform(
            lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
    }
    m_globalConstsCPU.lights[1].position = Vector3(0.0f, 1.1f, 2.0f) + lightDev;
    Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
    m_globalConstsCPU.lights[1].direction =
        focusPosition - m_globalConstsCPU.lights[1].position;
    m_globalConstsCPU.lights[1].direction.Normalize();

    // 그림자맵을 만들기 위한 시점
    for (int i = 0; i < MAX_LIGHTS; i++) {
        const auto &light = m_globalConstsCPU.lights[i];
        if (light.type & LIGHT_SHADOW) {

            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                up = Vector3(1.0f, 0.0f, 0.0f);

            // 그림자맵을 만들 때 필요
            Matrix lightViewRow = XMMatrixLookAtLH(
                light.position, light.position + light.direction, up);

            Matrix lightProjRow = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

            m_shadowGlobalConstsCPU[i].eyeWorld = light.position;
            m_shadowGlobalConstsCPU[i].view = lightViewRow.Transpose();
            m_shadowGlobalConstsCPU[i].proj = lightProjRow.Transpose();
            m_shadowGlobalConstsCPU[i].invProj =
                lightProjRow.Invert().Transpose();
            m_shadowGlobalConstsCPU[i].viewProj =
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

            D3D11Utils::UpdateBuffer(m_context, m_shadowGlobalConstsCPU[i],
                                     m_shadowGlobalConstsGPU[i]);

            // 그림자를 실제로 렌더링할 때 필요
            m_globalConstsCPU.lights[i].viewProj =
                m_shadowGlobalConstsCPU[i].viewProj;
            m_globalConstsCPU.lights[i].invProj =
                m_shadowGlobalConstsCPU[i].invProj;

            // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
            // 넣어주면 됩니다.
        }
    }
}

void AppBase::RenderDepthOnly() {
    m_context->OMSetRenderTargets(0, NULL, // DepthOnly라서 RTV 불필요
                                  m_depthOnlyDSV.Get());
    m_context->ClearDepthStencilView(m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
                                     1.0f, 0);
    AppBase::SetGlobalConsts(m_globalConstsGPU);
    for (const auto &model : m_basicList) {
        if (model->IsPostProcess() == false)
        {
            continue;
        }
        AppBase::SetPipelineState(model->GetDepthOnlyPSO());
        model->Render(m_context);
    }
    for (const auto& object : m_objectList) {
        if (object->GetBillboardModel() != nullptr)
        {
            continue;
        }
        AppBase::SetPipelineState(object->GetModel()->GetDepthOnlyPSO());
        object->Render(m_context);
    }
    for (const auto& object : m_objectList) {
        if (object->GetBillboardModel() == nullptr)
        {
            continue;
        }
        AppBase::SetPipelineState(object->GetModel()->GetDepthOnlyPSO());
        object->Render(m_context);
    }

    AppBase::SetPipelineState(Graphics::depthOnlyPSO);
    if (m_skybox)
        m_skybox->Render(m_context);
    if (m_mirror)
        m_mirror->Render(m_context);
}

void AppBase::RenderShadowMaps() {

    // 쉐도우 맵을 다른 쉐이더에서 SRV 해제
    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    m_context->PSSetShaderResources(15, 2, nulls);

    AppBase::SetShadowViewport(); // 그림자맵 해상도
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (m_globalConstsCPU.lights[i].type & LIGHT_SHADOW) {
            m_context->OMSetRenderTargets(0, NULL, // DepthOnly라서 RTV 불필요
                                          m_shadowDSVs[i].Get());
            m_context->ClearDepthStencilView(m_shadowDSVs[i].Get(),
                                             D3D11_CLEAR_DEPTH, 1.0f, 0);
            AppBase::SetGlobalConsts(m_shadowGlobalConstsGPU[i]);

            for (const auto &model : m_basicList) {
                if (model->m_castShadow && model->m_isVisible) {
                    AppBase::SetPipelineState(model->GetDepthOnlyPSO());
                    model->Render(m_context);
                }
            }
            for (const auto& object : m_objectList) {
                if (object->GetBillboardModel() != nullptr)
                {
                    continue;
                }
                shared_ptr<DModel> model = object->GetModel();
                if (model->m_castShadow && model->m_isVisible) {
                    AppBase::SetPipelineState(model->GetDepthOnlyPSO());
                    model->Render(m_context);
                }
            }
            for (const auto& object : m_objectList) {
                if (object->GetBillboardModel() == nullptr)
                {
                    continue;
                }
                shared_ptr<DModel> model = object->GetModel();
                if (model->m_castShadow && model->m_isVisible) {
                    AppBase::SetPipelineState(model->GetDepthOnlyPSO());
                    model->Render(m_context);
                }
            }

            if (m_mirror && m_mirror->m_castShadow)
                m_mirror->Render(m_context);
        }
    }
}

void AppBase::RenderOpaqueObjects() {
    // 다시 렌더링 해상도로 되돌리기
    AppBase::SetMainViewport();

    // 거울 1. 거울은 빼고 원래 대로 그리기
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_floatRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_floatRTV.GetAddressOf(),
                                  m_defaultDSV.Get());

    // 그림자맵들도 공용 텍스춰들 이후에 추가
    // 주의: 마지막 shadowDSV를 RenderTarget에서 해제한 후 설정
    vector<ID3D11ShaderResourceView *> shadowSRVs;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        shadowSRVs.push_back(m_shadowSRVs[i].Get());
    }
    m_context->PSSetShaderResources(15, UINT(shadowSRVs.size()),
                                    shadowSRVs.data());
    m_context->ClearDepthStencilView(
        m_defaultDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    AppBase::SetGlobalConsts(m_globalConstsGPU);

    // 스카이박스 그리기
    // 투명한 물체가 있어서 편의상 다른 물체들보다 먼저 그렸습니다.
    // 최적화를 하고 싶다면 투명한 물체들만 따로 마지막에 그리면 됩니다.
    AppBase::SetPipelineState(m_drawAsWire ? Graphics::skyboxWirePSO
                                           : Graphics::skyboxSolidPSO);
    m_skybox->Render(m_context);
    for (const auto& model : m_basicList) {

        AppBase::SetPipelineState(model->GetPSO(m_drawAsWire));
        model->Render(m_context);
    }
    for (const auto& object : m_objectList) {
        if (object->GetBillboardModel() != nullptr)
        {
            continue;
        }
        shared_ptr<DModel> model = object->GetModel();
        AppBase::SetPipelineState(model->GetPSO(m_drawAsWire));
        model->Render(m_context);
    }

    for (const auto& object : m_objectList) {
        if (object->GetBillboardModel() == nullptr)
        {
            continue;
        }
        shared_ptr<DModel> model = object->GetModel();
        AppBase::SetPipelineState(model->GetPSO(m_drawAsWire));
        model->Render(m_context);
    }
    // 거울 반사를 그릴 필요가 없으면 불투명 거울만 그리기
    if (m_mirrorAlpha == 1.0f && m_mirror) {
        AppBase::SetPipelineState(m_drawAsWire ? Graphics::defaultWirePSO
                                               : Graphics::defaultSolidPSO);
        m_mirror->Render(m_context);
    }

    // 노멀 벡터 그리기
    AppBase::SetPipelineState(Graphics::normalsPSO);
    for (auto &model : m_basicList) {
        if (model->m_drawNormals)
            model->RenderNormals(m_context);
    }

    AppBase::SetPipelineState(Graphics::boundingBoxPSO);
    if (AppBase::m_drawOBB) {
        for (auto &model : m_basicList) {
            model->RenderWireBoundingBox(m_context);
        }
        for (const auto& object : m_objectList) {
            shared_ptr<DModel> model = object->GetModel();
            model->RenderWireBoundingBox(m_context);
        }
    }
    if (AppBase::m_drawBS) {
        for (auto &model : m_basicList) {
            model->RenderWireBoundingSphere(m_context);
        }
        for (const auto& object : m_objectList) {
            shared_ptr<DModel> model = object->GetModel();
            model->RenderWireBoundingSphere(m_context);
        }
    }
}

void AppBase::RenderMirror() {

    if (m_mirrorAlpha < 1.0f && m_mirror) { // 거울 반사를 그려야 하는 상황

        // 거울 2. 거울 위치만 StencilBuffer에 1로 표기
        AppBase::SetPipelineState(Graphics::stencilMaskPSO);
        m_mirror->Render(m_context);

        // 거울 3. 거울 위치에 반사된 물체들을 렌더링
        AppBase::SetGlobalConsts(m_reflectGlobalConstsGPU);
        m_context->ClearDepthStencilView(m_defaultDSV.Get(), D3D11_CLEAR_DEPTH,
                                         1.0f, 0);

        for (auto &model : m_basicList) {
            AppBase::SetPipelineState(model->GetReflectPSO(m_drawAsWire));
            model->Render(m_context);
        }
        for (const auto& object : m_objectList) {
            if (object->GetBillboardModel() != nullptr)
            {
                continue;
            }
            AppBase::SetPipelineState(object->GetModel()->GetReflectPSO(m_drawAsWire));
            object->Render(m_context);
        }
        for (const auto& object : m_objectList) {
            if (object->GetBillboardModel() == nullptr)
            {
                continue;
            }
            AppBase::SetPipelineState(object->GetModel()->GetReflectPSO(m_drawAsWire));
            object->Render(m_context);
        }
        AppBase::SetPipelineState(m_drawAsWire
                                      ? Graphics::reflectSkyboxWirePSO
                                      : Graphics::reflectSkyboxSolidPSO);
        m_skybox->Render(m_context);

        // 거울 4. 거울 자체의 재질을 "Blend"로 그림
        AppBase::SetPipelineState(m_drawAsWire ? Graphics::mirrorBlendWirePSO
                                               : Graphics::mirrorBlendSolidPSO);
        AppBase::SetGlobalConsts(m_globalConstsGPU);
        m_mirror->Render(m_context);

    } // end of if (m_mirrorAlpha < 1.0f)
}

void AppBase::Render() {
    // Note : 사실 렌더링을 렌더스레드한테 요청한다면
    // 딱히 기다리면서 할 일이 없으니 그냥 메인 스레드에서 돌리도록한다.

    AppBase::SetMainViewport();

    // 공통으로 사용하는 샘플러들 설정
    m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());

    // 공통으로 사용할 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        m_envSRV.Get(), m_specularSRV.Get(), m_irradianceSRV.Get(),
        m_brdfSRV.Get()};
    m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());

    RenderDepthOnly();

    RenderShadowMaps();

    RenderOpaqueObjects();

    RenderMirror();
}


void AppBase::OnMouseMove(int mouseX, int mouseY) {

    m_mouseX = mouseX;
    m_mouseY = mouseY;

    // 마우스 커서의 위치를 NDC로 변환
    // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
    // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
    m_mouseNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_mouseNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;

    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
    m_mouseNdcX = std::clamp(m_mouseNdcX, -1.0f, 1.0f);
    m_mouseNdcY = std::clamp(m_mouseNdcY, -1.0f, 1.0f);

    // 카메라 시점 회전
    m_camera.UpdateMouse(m_mouseNdcX, m_mouseNdcY);
}

void AppBase::OnMouseClick(int mouseX, int mouseY) {

    m_mouseX = mouseX;
    m_mouseY = mouseY;

    m_mouseNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_mouseNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;
}

LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        // 화면 해상도가 바뀌면 SwapChain을 다시 생성
        if (m_swapChain) {

            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));

            // 윈도우가 Minimize 모드에서는 screenWidth/Height가 0
            if (m_screenWidth && m_screenHeight) {

                cout << "Resize SwapChain to " << m_screenWidth << " "
                     << m_screenHeight << endl;

                m_backBufferRTV.Reset();
                m_swapChain->ResizeBuffers(
                    0,                    // 현재 개수 유지
                    (UINT)LOWORD(lParam), // 해상도 변경
                    (UINT)HIWORD(lParam),
                    DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                    0);
                CreateBuffers();
                SetMainViewport();
                m_camera.SetAspectRatio(this->GetAspectRatio());

                m_postProcess.Initialize(
                    m_device, m_context, {m_postEffectsSRV, m_prevSRV},
                    {m_backBufferRTV}, m_screenWidth, m_screenHeight, 4);
            }
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
        if (m_activateActor!=nullptr)
        {
            if (m_activateActor->MsgProc(wParam, true))
            {
                return true;
            }
        }
        m_keyPressed[wParam] = true;
        if (wParam == VK_ESCAPE) { // ESC키 종료
            DestroyWindow(hwnd);
        }
        if (wParam == VK_SPACE) {
            m_lightRotate = !m_lightRotate;
        }
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
        if (wParam == 'C') { // c키 화면 캡쳐
            ComPtr<ID3D11Texture2D> backBuffer;
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
            D3D11Utils::WriteToPngFile(m_device, m_context, backBuffer,
                                       "captured.png");
        }
        if (wParam == 'P') { // 애니메이션 일시중지할 때 사용
            m_pauseAnimation = !m_pauseAnimation;
        }
        if (wParam == 'Z') { // 카메라 설정 화면에 출력
            m_camera.PrintView();
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

void AppBase::PostRender() {

    // Resolve MSAA texture
    m_context->ResolveSubresource(m_resolvedBuffer.Get(), 0,
                                  m_floatBuffer.Get(), 0,
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);

    // PostEffects (m_globalConstsGPU 사용)
    AppBase::SetMainViewport();
    AppBase::SetPipelineState(Graphics::postEffectsPSO);
    AppBase::SetGlobalConsts(m_globalConstsGPU);
    vector<ID3D11ShaderResourceView *> postEffectsSRVs = {m_resolvedSRV.Get(),
                                                          m_depthOnlySRV.Get()};
    m_context->PSSetShaderResources(20, // 주의: Startslop 20
                                    UINT(postEffectsSRVs.size()),
                                    postEffectsSRVs.data());
    m_context->OMSetRenderTargets(1, m_postEffectsRTV.GetAddressOf(), NULL);
    m_context->PSSetConstantBuffers(5, // register(b5)
                                    1, m_postEffectsConstsGPU.GetAddressOf());
    m_screenSquare->Render(m_context);

    ID3D11ShaderResourceView *nulls[2] = {0, 0};
    m_context->PSSetShaderResources(20, 2, nulls);

    // 후처리 (블룸 같은 순수 이미지 처리)
    AppBase::SetPipelineState(Graphics::postProcessingPSO);
    m_postProcess.Render(m_context);

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    m_context->CopyResource(
        m_prevBuffer.Get(),
        backBuffer.Get()); // 모션 블러 효과를 위해 렌더링 결과 보관
}

void AppBase::InitCubemaps(wstring basePath, wstring envFilename,
                           wstring specularFilename, wstring irradianceFilename,
                           wstring brdfFilename) {

    // BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
    D3D11Utils::CreateDDSTexture(m_device, (basePath + envFilename).c_str(),
                                 true, m_envSRV);
    D3D11Utils::CreateDDSTexture(
        m_device, (basePath + specularFilename).c_str(), true, m_specularSRV);
    D3D11Utils::CreateDDSTexture(m_device,
                                 (basePath + irradianceFilename).c_str(), true,
                                 m_irradianceSRV);
    D3D11Utils::CreateDDSTexture(m_device, (basePath + brdfFilename).c_str(),
                                 false, m_brdfSRV);
}

// 여러 물체들이 공통적으료 사용하는 Const 업데이트
void AppBase::UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                                    const Matrix &viewRow,
                                    const Matrix &projRow, const Matrix &refl) {

    m_globalConstsCPU.globalTime += dt;
    m_globalConstsCPU.eyeWorld = eyeWorld;
    m_globalConstsCPU.view = viewRow.Transpose();
    m_globalConstsCPU.proj = projRow.Transpose();
    m_globalConstsCPU.invProj = projRow.Invert().Transpose();
    m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    m_globalConstsCPU.invView = viewRow.Invert().Transpose();

    // 그림자 렌더링에 사용
    m_globalConstsCPU.invViewProj = m_globalConstsCPU.viewProj.Invert();

    m_reflectGlobalConstsCPU = m_globalConstsCPU;
    memcpy(&m_reflectGlobalConstsCPU, &m_globalConstsCPU,
           sizeof(m_globalConstsCPU));
    m_reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
    m_reflectGlobalConstsCPU.viewProj = (refl * viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    m_reflectGlobalConstsCPU.invViewProj =
        m_reflectGlobalConstsCPU.viewProj.Invert();

    D3D11Utils::UpdateBuffer(m_context, m_globalConstsCPU, m_globalConstsGPU);
    D3D11Utils::UpdateBuffer(m_context, m_reflectGlobalConstsCPU,
                             m_reflectGlobalConstsGPU);
}

void AppBase::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0m_envSRV
    m_context->VSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    m_context->GSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
}

void AppBase::CreateDepthBuffers() {

    // DepthStencilView 만들기
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_screenWidth;
    desc.Height = m_screenHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if (m_useMSAA && m_numQualityLevels > 0) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateDepthStencilView(
        depthStencilBuffer.Get(), NULL, m_defaultDSV.GetAddressOf()));

    // Depth 전용
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_depthOnlyBuffer.GetAddressOf()));

    // 그림자 Buffers (Depth 전용)
    desc.Width = m_shadowWidth;
    desc.Height = m_shadowHeight;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateTexture2D(
            &desc, NULL, m_shadowBuffers[i].GetAddressOf()));
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(m_device->CreateDepthStencilView(
        m_depthOnlyBuffer.Get(), &dsvDesc, m_depthOnlyDSV.GetAddressOf()));

    // 그림자 DSVs
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(
            m_device->CreateDepthStencilView(m_shadowBuffers[i].Get(), &dsvDesc,
                                             m_shadowDSVs[i].GetAddressOf()));
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_depthOnlyBuffer.Get(), &srvDesc, m_depthOnlySRV.GetAddressOf()));

    // 그림자 SRVs
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateShaderResourceView(
            m_shadowBuffers[i].Get(), &srvDesc,
            m_shadowSRVs[i].GetAddressOf()));
    }
}

void AppBase::SetPipelineState(const GraphicsPSO &pso) {

    m_context->VSSetShader(pso.m_vertexShader.Get(), 0, 0);
    m_context->PSSetShader(pso.m_pixelShader.Get(), 0, 0);
    m_context->HSSetShader(pso.m_hullShader.Get(), 0, 0);
    m_context->DSSetShader(pso.m_domainShader.Get(), 0, 0);
    m_context->GSSetShader(pso.m_geometryShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);
    m_context->IASetInputLayout(pso.m_inputLayout.Get());
    m_context->RSSetState(pso.m_rasterizerState.Get());
    m_context->OMSetBlendState(pso.m_blendState.Get(), pso.m_blendFactor,
                               0xffffffff);
    m_context->OMSetDepthStencilState(pso.m_depthStencilState.Get(),
                                      pso.m_stencilRef);
    m_context->IASetPrimitiveTopology(pso.m_primitiveTopology);
}

void AppBase::SetPipelineState(const ComputePSO &pso) {
    m_context->VSSetShader(NULL, 0, 0);
    m_context->PSSetShader(NULL, 0, 0);
    m_context->HSSetShader(NULL, 0, 0);
    m_context->DSSetShader(NULL, 0, 0);
    m_context->GSSetShader(NULL, 0, 0);
    m_context->CSSetShader(pso.m_computeShader.Get(), 0, 0);
}

shared_ptr<Model> AppBase::PickClosest(const Ray &pickingRay, float &minDist) {
    minDist = 1e5f;
    shared_ptr<Model> minModel = nullptr;
    for (auto &model : m_basicList) {
        float dist = 0.0f;
        if (model->m_isPickable &&
            pickingRay.Intersects(model->m_boundingSphere, dist) &&
            dist < minDist) {
            minModel = model;
            minDist = dist;
        }
    }
    return minModel;
}

void AppBase::SelectClosestActor(const Ray& pickingRay, float& minDist) {
    minDist = 1e5f;
    for (auto& object : m_objectList) {
        std::shared_ptr<Actor> actor = std::dynamic_pointer_cast<Actor>(object);
        if (actor == nullptr)
        {
            continue;
        }
        float dist = 0.0f;
        std::shared_ptr<DModel> model = object->GetModel();

        if (object->IsPickable() &&
            pickingRay.Intersects(model->m_boundingSphere, dist) &&
            dist < minDist) {
            m_activateActor = actor;
            minDist = dist;
        }
    }
}

void AppBase::ProcessMouseControl() {

    static shared_ptr<Model> activeModel = nullptr;
    static float prevRatio = 0.0f;
    static Vector3 prevPos(0.0f);
    static Vector3 prevVector(0.0f);

    // 적용할 회전과 이동 초기화
    Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 dragTranslation(0.0f);
    Vector3 pickPoint(0.0f);
    float dist = 0.0f;

    // 사용자가 두 버튼 중 하나만 누른다고 가정합니다.
    if (m_leftButton || m_rightButton) {
        const Matrix viewRow = m_camera.GetViewRow();
        const Matrix projRow = m_camera.GetProjRow();
        const Vector3 ndcNear = Vector3(m_mouseNdcX, m_mouseNdcY, 0.0f);
        const Vector3 ndcFar = Vector3(m_mouseNdcX, m_mouseNdcY, 1.0f);
        const Matrix invProjView = (viewRow * projRow).Invert();
        const Vector3 worldNear = Vector3::Transform(ndcNear, invProjView);
        const Vector3 worldFar = Vector3::Transform(ndcFar, invProjView);
        Vector3 dir = worldFar - worldNear;
        dir.Normalize();
        const Ray curRay = SimpleMath::Ray(worldNear, dir);

        SelectClosestActor(curRay, dist);

        // 이전 프레임에서 아무 물체도 선택되지 않았을 경우에는 새로 선택
        if (!activeModel) {
            auto newModel = AppBase::PickClosest(curRay, dist);
            if (newModel) {
                cout << "Newly selected model: " << newModel->m_name << endl;
                activeModel = newModel;
                m_pickedModel = newModel; // GUI 조작용 포인터
                pickPoint = curRay.position + dist * curRay.direction;
                if (m_leftButton) { // 왼쪽 버튼 회전 준비
                    prevVector =
                        pickPoint - activeModel->m_boundingSphere.Center;
                    prevVector.Normalize();
                } else { // 오른쪽 버튼 이동 준비
                    m_dragStartFlag = false;
                    prevRatio = dist / (worldFar - worldNear).Length();
                    prevPos = pickPoint;
                }
            }
        } else {                // 이미 선택된 물체가 있었던 경우
            if (m_leftButton) { // 왼쪽 버튼으로 계속 회전
                if (curRay.Intersects(activeModel->m_boundingSphere, dist)) {
                    pickPoint = curRay.position + dist * curRay.direction;
                } else {
                    // 바운딩 스피어에 가장 가까운 점을 찾기
                    Vector3 c =
                        activeModel->m_boundingSphere.Center - worldNear;
                    Vector3 centerToRay = dir.Dot(c) * dir - c;
                    pickPoint =
                        c +
                        centerToRay *
                            std::clamp(activeModel->m_boundingSphere.Radius /
                                           centerToRay.Length(),
                                       0.0f, 1.0f);
                    pickPoint += worldNear;
                }

                Vector3 currentVector =
                    pickPoint - activeModel->m_boundingSphere.Center;
                currentVector.Normalize();
                float theta = acos(prevVector.Dot(currentVector));
                if (theta > 3.141592f / 180.0f * 3.0f) {
                    Vector3 axis = prevVector.Cross(currentVector);
                    axis.Normalize();
                    q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                                                                    theta);
                    prevVector = currentVector;
                }

            } else { // 오른쪽 버튼으로 계속 이동
                Vector3 newPos = worldNear + prevRatio * (worldFar - worldNear);
                if ((newPos - prevPos).Length() > 1e-3) {
                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
                pickPoint = newPos; // Cursor sphere 그려질 위치
            }
        }
    } else {
        // 버튼에서 손을 땠을 경우에는 움직일 모델은 nullptr로 설정
        activeModel = nullptr;

        // m_pickedModel은 GUI 조작을 위해 마우스에서 손을 떼도 nullptr로
        // 설정하지 않음
    }

    // Cursor sphere 그리기
    if (activeModel) {
        Vector3 translation = activeModel->m_worldRow.Translation();
        activeModel->m_worldRow.Translation(Vector3(0.0f));
        activeModel->UpdateWorldRow(
            activeModel->m_worldRow * Matrix::CreateFromQuaternion(q) *
            Matrix::CreateTranslation(dragTranslation + translation));
        activeModel->m_boundingSphere.Center =
            activeModel->m_worldRow.Translation();

        // 충돌 지점에 작은 구 그리기
        //m_cursorSphere->m_isVisible = true;
        //m_cursorSphere->UpdateWorldRow(Matrix::CreateTranslation(pickPoint));
    } else {
        //m_cursorSphere->m_isVisible = false;
    }
}

bool AppBase::InitMainWindow() {

    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"Daerim'sGTA", // lpszClassName, L-string
                     NULL};

    if (!RegisterClassEx(&wc)) {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    RECT wr = {0, 0, m_screenWidth, m_screenHeight};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
    m_mainWindow = CreateWindow(wc.lpszClassName, L"Daerim'sGTA",
                                WS_OVERLAPPEDWINDOW,
                                100, // 윈도우 좌측 상단의 x 좌표
                                100, // 윈도우 좌측 상단의 y 좌표
                                wr.right - wr.left, // 윈도우 가로 방향 해상도
                                wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                                NULL, NULL, wc.hInstance, NULL);

    if (!m_mainWindow) {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_mainWindow);

    return true; 
}

bool AppBase::InitDirect3D() {
    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
    // const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format = m_backBufferFormat;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT |
                     DXGI_USAGE_UNORDERED_ACCESS; // Compute Shader
    sd.OutputWindow = m_mainWindow;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가
    // 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(), &featureLevel, m_context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        cout << "D3D Feature Level 11 unsupported." << endl;
        return false;
    }


    ID3D11Debug* debugDevice = nullptr;
    if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice)))) {
        debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debugDevice->Release();
    }


    Graphics::InitCommonStates(m_device);

    CreateBuffers();

    SetMainViewport();

    // 공통으로 쓰이는 ConstBuffers
    D3D11Utils::CreateConstBuffer(m_device, m_globalConstsCPU,
                                  m_globalConstsGPU);
    D3D11Utils::CreateConstBuffer(m_device, m_reflectGlobalConstsCPU,
                                  m_reflectGlobalConstsGPU);

    // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
    for (int i = 0; i < MAX_LIGHTS; i++) {
        D3D11Utils::CreateConstBuffer(m_device, m_shadowGlobalConstsCPU[i],
                                      m_shadowGlobalConstsGPU[i]);
    }

    // 후처리 효과용 ConstBuffer
    D3D11Utils::CreateConstBuffer(m_device, m_postEffectsConstsCPU,
                                  m_postEffectsConstsGPU);

    return true;
}

bool AppBase::InitGUI() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_mainWindow)) {
        return false;
    }

    return true;
}

void AppBase::SetMainViewport() {

    // Set the viewport
    ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = float(m_screenWidth);
    m_screenViewport.Height = float(m_screenHeight);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &m_screenViewport);
}

void AppBase::SetShadowViewport() {

    // Set the viewport
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = float(m_shadowWidth);
    shadowViewport.Height = float(m_shadowHeight);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &shadowViewport);
}

void AppBase::ComputeShaderBarrier() {

    // 참고: BreadcrumbsDirectX-Graphics-Samples (DX12)
    // void CommandContext::InsertUAVBarrier(GpuResource & Resource, bool
    // FlushImmediate)

    // 예제들에서 최대 사용하는 SRV, UAV 갯수가 6개
    ID3D11ShaderResourceView *nullSRV[6] = {
        0,
    };
    m_context->CSSetShaderResources(0, 6, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[6] = {
        0,
    };
    m_context->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
}

void AppBase::KillObjects()
{
    for (int i = m_objectList.size()-1; i >=0 ; i--)
    {
        if (m_objectList[i]->IsPendingKill())
        {
            DaerimsEngineBase::GetInstance().RemoveRigidBody(m_objectList[i]->GetPhysicsBody());
            m_objectList.erase(m_objectList.begin() + i);
        }
    }
}

void AppBase::CreateBuffers() {

    // 레스터화 -> float/depthBuffer(MSAA) -> resolved -> backBuffer

    // BackBuffer는 화면으로 최종 출력 (SRV는 불필요)
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        backBuffer.Get(), NULL, m_backBufferRTV.GetAddressOf()));

    // FLOAT MSAA RenderTargetView/ShaderResourceView
    ThrowIfFailed(m_device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &m_numQualityLevels));

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // 이전 프레임 저장용
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_prevBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_prevBuffer.Get(), NULL,
                                                   m_prevRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(m_prevBuffer.Get(), NULL,
                                                     m_prevSRV.GetAddressOf()));

    desc.MipLevels = desc.ArraySize = 1;

    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    if (m_useMSAA && m_numQualityLevels) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }

    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_floatBuffer.GetAddressOf()));

    ThrowIfFailed(m_device->CreateRenderTargetView(m_floatBuffer.Get(), NULL,
                                                   m_floatRTV.GetAddressOf()));

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
                     D3D11_BIND_UNORDERED_ACCESS;
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_resolvedBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateTexture2D(
        &desc, NULL, m_postEffectsBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_resolvedBuffer.Get(), NULL, m_resolvedSRV.GetAddressOf()));

    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_postEffectsBuffer.Get(), NULL, m_postEffectsSRV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_resolvedBuffer.Get(), NULL, m_resolvedRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_postEffectsBuffer.Get(), NULL, m_postEffectsRTV.GetAddressOf()));

    CreateDepthBuffers();

    m_postProcess.Initialize(m_device, m_context, {m_postEffectsSRV, m_prevSRV},
                             {m_backBufferRTV}, m_screenWidth, m_screenHeight,
                             4);
}

} // namespace hlab