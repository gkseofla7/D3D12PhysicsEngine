#include "Ex1404_StructuredBuffer.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1404_StructuredBuffer::Ex1404_StructuredBuffer() : AppBase() {}

bool Ex1404_StructuredBuffer::Initialize() {

    cout << "Ex1404_StructuredBuffer::Initialize()" << endl;

    // 이 예제에서는 ComputeShader가 BackBuffer를 직접 사용하지 않음
    // AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    m_screenWidth = 1280;
    m_screenHeight = 1280;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화
    m_particles.m_cpu.resize(25600);

    std::mt19937 gen(0);
    std::uniform_real_distribution<float> dp(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dc(0.8f, 1.0f);
    for (auto &p : m_particles.m_cpu) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = Vector3(dc(gen), dc(gen), dc(gen));
    }

    m_particles.Initialize(m_device);

    // 주의: Vertex Shader에서 Vertex 정보 미사용

    // Vertex Shader
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1404_StructuredBufferVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);

    // Pixel Shader
    D3D11Utils::CreatePixelShader(m_device, L"Ex1404_StructuredBufferPS.hlsl",
                                  m_pixelShader);

    // Compute shader
    D3D11Utils::CreateComputeShader(m_device, L"Ex1404_StructuredBufferCS.hlsl",
                                    m_computeShader);

    return true;
}

void Ex1404_StructuredBuffer::Update(float dt) {
    // 입자들의 위치를 바꿔주는 작업도 GPU에서 진행
}

void Ex1404_StructuredBuffer::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    // Compute Shader에서 Particle 위치 변경
    m_context->CSSetUnorderedAccessViews(0, 1, m_particles.GetAddressOfUAV(),
                                         NULL);
    m_context->CSSetShader(m_computeShader.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_particles.m_cpu.size() / 256.0f)), 1, 1);
    AppBase::ComputeShaderBarrier();

    // timer.End(m_context); // GPU: 0.00672, CPU: 15.6909

    // Vertex/Particle shader에서 Rendering

    // 주의: m_context->IASetInputLayout(.) 미사용
    // 주의: m_context->IASetVertexBuffers(.) 미사용

    AppBase::SetMainViewport();
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), NULL);
    m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
    m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);

    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // StructuredBuffer<Particle> particles : register(t0);
    m_context->VSSetShaderResources(0, 1, m_particles.GetAddressOfSRV());
    m_context->Draw(UINT(m_particles.m_cpu.size()), 0);
}

void Ex1404_StructuredBuffer::UpdateGUI() {}

} // namespace hlab