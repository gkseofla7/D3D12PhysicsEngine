#include "Ex1406_DensityField.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1406_DensityField::Ex1406_DensityField() : AppBase() {}

bool Ex1406_DensityField::Initialize() {

    cout << "Ex1406_DensityField::Initialize()" << endl;

    // ComputeShader에서 Backbuffer를 사용
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    m_screenWidth = 1024;
    m_screenHeight = 1024;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화
    m_particles.m_cpu.resize(256);

    std::vector<Vector3> rainbow = {
        {1.0f, 0.0f, 0.0f},  // Red
        {1.0f, 0.65f, 0.0f}, // Orange
        {1.0f, 1.0f, 0.0f},  // Yellow
        {0.0f, 1.0f, 0.0f},  // Green
        {0.0f, 0.0f, 1.0f},  // Blue
        {0.3f, 0.0f, 0.5f},  // Indigo
        {0.5f, 0.0f, 1.0f}   // Violet/Purple
    };

    std::mt19937 gen(0);
    std::uniform_real_distribution<float> dp(-1.0f, 1.0f);
    std::uniform_int_distribution<size_t> dc(0, rainbow.size() - 1);
    for (auto &p : m_particles.m_cpu) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = rainbow[dc(gen)];
    }

    m_particles.Initialize(m_device);

    // 주의: Vertex Shader에서 Vertex 정보 미사용

    // VS는 이전 예제와 동일
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1404_StructuredBufferVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1406_SpritePS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1406_DensitySourcingCS.hlsl",
                                    m_densitySourcingCS);
    D3D11Utils::CreateComputeShader(
        m_device, L"Ex1406_DensityDissipationCS.hlsl", m_densityDissipationCS);

    m_densityTex.Initialize(m_device, m_screenWidth, m_screenHeight,
                            DXGI_FORMAT_R16G16B16A16_FLOAT);

    D3D11Utils::CreateGeometryShader(m_device, L"Ex1406_SpriteGS.hlsl",
                                     m_spriteGS);

    return true;
}

void Ex1406_DensityField::Update(float dt) {
    // 입자들의 위치를 바꿔주는 작업도 GPU에서 진행
}

void Ex1406_DensityField::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    DissipateDensity();
    AdvectParticles();
    DrawSprites();

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    m_context->CopyResource(backBuffer.Get(), m_densityTex.GetTexture());

    // timer.End(m_context);
}

void Ex1406_DensityField::DissipateDensity() { // Density Field의 Dissipation
                                               // (Compute Shader)
    m_context->CSSetUnorderedAccessViews(0, 1, m_densityTex.GetAddressOfUAV(),
                                         NULL);
    m_context->CSSetShader(m_densityDissipationCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_screenWidth / 32.0f)),
                        UINT(ceil(m_screenHeight / 32.0f)), 1);
    AppBase::ComputeShaderBarrier();
}

void Ex1406_DensityField::AdvectParticles() {

    // 입자들이 Density field에 색상 추가 sourcing (Compute Shader)
    // 주의: ComputeShader버전은 Thread-safe 하지 않음 -> Draw() 사용

    ID3D11UnorderedAccessView *uavs2[2] = {m_particles.GetUAV(),
                                           m_densityTex.GetUAV()};
    m_context->CSSetUnorderedAccessViews(0, 2, uavs2, NULL);
    m_context->CSSetShader(m_densitySourcingCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_particles.m_cpu.size() / 256.0f)), 1, 1);
    AppBase::ComputeShaderBarrier();
}

void Ex1406_DensityField::DrawSprites() {

    // Geometry Shader로 Particle Sprites 그리기

    AppBase::SetMainViewport();

    // 시간에 따른 누적 효과(모션 블러)를 원할때는 Clear 생략
    // const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    // m_context->ClearRenderTargetView(m_densityTex.GetRTV(), clearColor);

    m_context->OMSetRenderTargets(1, m_densityTex.GetAddressOfRTV(), NULL);
    m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
    m_context->GSSetShader(m_spriteGS.Get(), 0, 0);
    m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);
    // TODO: m_context->OMSetBlendState(...)

    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    m_context->VSSetShaderResources(0, 1, m_particles.GetAddressOfSRV());
    m_context->Draw(UINT(m_particles.m_cpu.size()), 0);
}

void Ex1406_DensityField::UpdateGUI() {}

} // namespace hlab