#include "Ex1405_ConsumeAppendBuffer.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1405_ConsumeAppendBuffer::Ex1405_ConsumeAppendBuffer() : AppBase() {}

bool Ex1405_ConsumeAppendBuffer::Initialize() {

    cout << "Ex1405_ConsumeAppendBuffer::Initialize()" << endl;

    m_screenWidth = 1280;
    m_screenHeight = 1280;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화
    m_consume.m_cpu.resize(25600);

    std::mt19937 gen(0);
    std::uniform_real_distribution<float> dp(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dc(0.0f, 1.0f);
    for (auto &p : m_consume.m_cpu) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = Vector3(dc(gen), dc(gen), dc(gen));
    }

    m_consume.Initialize(m_device);

    // 초기 데이터가 없더라도 사이즈 지정 필요
    m_append.m_cpu.resize(m_consume.m_cpu.size());
    m_append.Initialize(m_device);

    // Structures Count 저장용, 4바이트 UINT 하나
    m_countStaging.Initialize(m_device, {0});

    // VS, PS는 StructuredBuffer 예제와 동일, CS는 다름
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1404_StructuredBufferVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1404_StructuredBufferPS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1405_ConsumeAppendCS.hlsl",
                                    m_computeShader);

    return true;
}

void Ex1405_ConsumeAppendBuffer::Update(float dt) {
    // 입자들의 위치를 바꿔주는 작업도 GPU에서 진행
}

void Ex1405_ConsumeAppendBuffer::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    // Compute Shader에서 Particle 위치 변경
    ID3D11UnorderedAccessView *uavs[2] = {m_consume.GetUAV(),
                                          m_append.GetUAV()};

    // {m_consumeUAV의 시작 크기, m_appendUAV의 시작 크기}
    UINT initCounts[2] = {UINT(m_consume.m_cpu.size()), 0}; // <- 중요

    m_context->CSSetUnorderedAccessViews(0, 2, uavs, initCounts);
    m_context->CSSetShader(m_computeShader.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_consume.m_cpu.size() / 256.0f)), 1, 1);
    AppBase::ComputeShaderBarrier();

    // timer.End(m_context); // GPU: 0.008448, CPU: 14.7688

    // 주의: Dispatch()로 인해 내부적으로 실행되는 회수가 버퍼 크기보다 크면
    // Consume이 제대로 이뤄지지 않아서 문제가 생깁니다.
    // 예시: 버퍼 크기는 300개, numthreads(256, 1, 1)일때 Dispatch(2, 1, 1)로
    // 실행하면 쉐이더 내부에서 512-300=212 번의 Consume/Append()이
    // 추가로 실행됩니다.

    // AppendUAV 개수 복사
    m_context->CopyStructureCount(m_countStaging.m_gpu.Get(), 0,
                                  m_append.GetUAV());
    m_countStaging.Download(m_context);
    uint32_t appendCount = m_countStaging.m_cpu[0];
    cout << "AppendBuffer count: " << appendCount << endl;

    // ConsumeUAV 개수 복사
    m_context->CopyStructureCount(m_countStaging.m_gpu.Get(), 0,
                                  m_consume.GetUAV());
    m_countStaging.Download(m_context);

    AppBase::SetMainViewport();
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), NULL);
    m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
    m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);

    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // VS에서 StructuredBuffer<Particle> particles : register(t0);
    m_context->VSSetShaderResources(0, 1, m_append.GetAddressOfSRV());

    m_context->Draw(UINT(appendCount), 0); // <- AppendCount 만큼만 그리기

    // DrawIndexedInstancedIndirect()를 사용하면 AppendCount를 CPU로 복사하지
    // 않아도 됩니다.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstancedindirect

    ID3D11ShaderResourceView *nullSRVs[1] = {NULL};
    m_context->VSSetShaderResources(0, 1, nullSRVs);

    // 복사 또는 CSSetUAVs에서 넣어주는 순서 바꾸기
    // m_context->CopyResource(m_consume.GetBuffer(), m_append.GetBuffer());
    hlab::swap(m_consume, m_append);
}

void Ex1405_ConsumeAppendBuffer::UpdateGUI() {}

} // namespace hlab