#include "Ex1602_CurlNoise.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1602_CurlNoise::Ex1602_CurlNoise() : AppBase() {}

bool Ex1602_CurlNoise::Initialize() {
    cout << "Ex1602_CurlNoise::Initialize()" << endl;

    AppBase::m_screenWidth = 1024;
    AppBase::m_screenHeight = 1024;
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화
    m_particlesCPU.resize(m_screenWidth);

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
    for (auto &p : m_particlesCPU) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = rainbow[dc(gen)];
    }

    D3D11Utils::CreateStructuredBuffer(
        m_device, UINT(m_particlesCPU.size()), sizeof(Particle),
        m_particlesCPU.data(), m_particlesGPU, m_particlesSRV, m_particlesUAV);

    // Vertex Shader
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1404_StructuredBufferVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1406_SpritePS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateComputeShader(
        m_device, L"Ex1406_DensityDissipationCS.hlsl", m_densityDissipationCS);
    D3D11Utils::CreateGeometryShader(m_device, L"Ex1406_SpriteGS.hlsl",
                                     m_spriteGS);

    m_density.Initialize(m_device, m_screenWidth, m_screenHeight,
                         DXGI_FORMAT_R16G16B16A16_FLOAT);

    D3D11Utils::CreateComputeShader(m_device, L"Ex1602_CurlNoiseCS.hlsl",
                                    m_curlNoiseCS);

    return true;
}

bool Ex1602_CurlNoise::InitScene() {
    cout << "Ex1602_CurlNoise::InitScene()" << endl;
    AppBase::InitScene();
    return true;
}

void Ex1602_CurlNoise::Update(float dt) {

    m_context->CSSetUnorderedAccessViews(0, 1, m_density.GetAddressOfUAV(),
                                         NULL);
    m_context->CSSetShader(m_densityDissipationCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_screenWidth / 32.0f)),
                        UINT(ceil(m_screenHeight / 32.0f)), 1);
    AppBase::ComputeShaderBarrier();

    m_context->CSSetUnorderedAccessViews(0, 1, m_density.GetAddressOfUAV(),
                                         NULL);
    m_context->CSSetUnorderedAccessViews(1, 1, m_particlesUAV.GetAddressOf(),
                                         NULL);
    m_context->CSSetShader(m_curlNoiseCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_screenWidth / 32.0f)),
                        UINT(ceil(m_screenHeight / 32.0f)), 1);
    ComputeShaderBarrier();
} 

void Ex1602_CurlNoise::Render() {

    AppBase::SetMainViewport();

    // 시간에 따른 누적 효과(모션 블러)를 원할때는 Clear 생략
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(*m_density.GetAddressOfRTV(),
                                     clearColor);

    m_context->OMSetRenderTargets(1, m_density.GetAddressOfRTV(), NULL);
    m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
    m_context->GSSetShader(m_spriteGS.Get(), 0, 0);
    m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);
    const float blendColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_context->OMSetBlendState(Graphics::accumulateBS.Get(), blendColor,
                               0xffffffff);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    m_context->VSSetShaderResources(0, 1, m_particlesSRV.GetAddressOf());
    m_context->Draw(UINT(m_particlesCPU.size()), 0);
     
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    m_context->CopyResource(backBuffer.Get(), m_density.GetTexture());
}

void Ex1602_CurlNoise::UpdateGUI() { AppBase::UpdateGUI(); }

} // namespace hlab