#pragma once

#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "ComputePSO.h"
#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GraphicsPSO.h"
#include "Model.h"
#include "Actor.h"
#include "PostProcess.h"
#include "Timer.h"
//#include "Delegate.h"

namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Ray;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

class AppBase {
  public:
    AppBase();
    virtual ~AppBase();

    int Run();
    float GetAspectRatio() const;

    virtual bool Initialize();
    virtual bool InitScene();
    virtual void UpdateGUI() {}
    virtual void Update(float dt);
    virtual void UpdateLights(float dt);
    virtual void RenderDepthOnly();
    virtual void RenderShadowMaps();
    virtual void RenderOpaqueObjects();
    virtual void RenderMirror();
    virtual void Render();

    virtual void OnMouseMove(int mouseX, int mouseY);
    virtual void OnMouseClick(int mouseX, int mouseY);
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void PostRender();

    void InitCubemaps(wstring basePath, wstring envFilename,
                      wstring specularFilename, wstring irradianceFilename,
                      wstring brdfFilename);
    void UpdateGlobalConstants(const float &dt, const Vector3 &eyeWorld,
                               const Matrix &viewRow, const Matrix &projRow,
                               const Matrix &refl = Matrix());
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);

    void CreateDepthBuffers();
    void SetPipelineState(const GraphicsPSO &pso);
    void SetPipelineState(const ComputePSO &pso);
    shared_ptr<Model> PickClosest(const Ray &pickingRay, float &minDist);
    void ProcessMouseControl();

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();
    void CreateBuffers();
    void SetMainViewport();
    void SetShadowViewport();
    void ComputeShaderBarrier();

  public:
    // 변수 이름 붙이는 규칙은 VS DX11/12 기본 템플릿을 따릅니다.
    // 변수 이름을 줄이기 위해 d3d는 생략했습니다.
    // 예: m_d3dDevice -> m_device

    int m_screenWidth;
    int m_screenHeight;
    HWND m_mainWindow;
    bool m_useMSAA = true;
    UINT m_numQualityLevels = 0;
    bool m_drawAsWire = false;
    bool m_drawOBB = false; // Draw Object Oriented Bounding Box
    bool m_drawBS = false;  // Draw Bounding Sphere

    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

    // 삼각형 레스터화 -> float(MSAA) -> resolved(No MSAA)
    // -> 후처리(블룸, 톤매핑) -> backBuffer(최종 SwapChain Present)

    ComPtr<ID3D11Texture2D> m_floatBuffer;
    ComPtr<ID3D11Texture2D> m_resolvedBuffer;
    ComPtr<ID3D11Texture2D> m_postEffectsBuffer;
    ComPtr<ID3D11Texture2D> m_prevBuffer; // 간단한 모션 블러 효과
    ComPtr<ID3D11RenderTargetView> m_floatRTV;
    ComPtr<ID3D11RenderTargetView> m_resolvedRTV;
    ComPtr<ID3D11RenderTargetView> m_postEffectsRTV;
    ComPtr<ID3D11RenderTargetView> m_prevRTV;
    ComPtr<ID3D11ShaderResourceView> m_resolvedSRV;
    ComPtr<ID3D11ShaderResourceView> m_postEffectsSRV;
    ComPtr<ID3D11ShaderResourceView> m_prevSRV;

    // Depth buffer 관련
    ComPtr<ID3D11Texture2D> m_depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_depthOnlyDSV;
    ComPtr<ID3D11DepthStencilView> m_defaultDSV;
    ComPtr<ID3D11ShaderResourceView> m_depthOnlySRV;

    // Shadow maps
    int m_shadowWidth = 1280;
    int m_shadowHeight = 1280;
    ComPtr<ID3D11Texture2D> m_shadowBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowDSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> m_shadowSRVs[MAX_LIGHTS];

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라 클래스 추가
    Camera m_camera;
    bool m_keyPressed[256] = {
        false,
    };

    bool m_leftButton = false;
    bool m_rightButton = false;
    bool m_dragStartFlag = false;

    // 마우스 커서 위치 저장 (Picking에 사용)
    float m_mouseNdcX = 0.0f;
    float m_mouseNdcY = 0.0f;
    float m_wheelDelta = 0.0f;
    int m_mouseX = -1;
    int m_mouseY = -1;

    // 렌더링 -> PostEffects -> PostProcess
    PostEffectsConstants m_postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> m_postEffectsConstsGPU;

    PostProcess m_postProcess;

    // 다양한 Pass들을 더 간단히 구현하기 위해 ConstBuffer들 분리
    GlobalConstants m_globalConstsCPU;
    GlobalConstants m_reflectGlobalConstsCPU;
    GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_globalConstsGPU;
    ComPtr<ID3D11Buffer> m_reflectGlobalConstsGPU;
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];

    // 공통으로 사용하는 텍스춰들
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

    bool m_lightRotate = false;
    bool m_pauseAnimation = false;

    // 여러 예제들 공용
    shared_ptr<Model> m_screenSquare; // PostEffect에 사용
    shared_ptr<Model> m_skybox;
    shared_ptr<Model> m_pickedModel;
    shared_ptr<Model> m_lightSphere[MAX_LIGHTS];
    shared_ptr<Model> m_cursorSphere;
    shared_ptr<Model> m_mirror; // 거울은 별도로 그림
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // Opacity

    // 거울이 아닌 물체들의 리스트 (for문으로 그리기 위함)
    vector<shared_ptr<Model>> m_basicList;
    vector<shared_ptr<Actor>> m_actorList;
    
    //셋팅 했을 경우 해당 액터에 키가 바인딩 돼있다면 액터가 먹음
    shared_ptr<Actor> m_activateActor;
};

} // namespace hlab