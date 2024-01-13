#include "Ex1401_Basic.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1401_Basic::Ex1401_Basic() : AppBase() {}

bool Ex1401_Basic::Initialize() {

    cout << "Ex1401_Basic::Initialize()" << endl;

    // ComputeShader에서 BackBuffer를 사용하기 위해서 FLOAT로 설정
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    if (!AppBase::Initialize())
        return false;

    // 백버퍼의 텍스춰 가져오기 (백버퍼도 FLOAT 사용)
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // 백버퍼의 UAV 만들기
    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    uavDesc.Format = desc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    ThrowIfFailed(m_device->CreateUnorderedAccessView(
        backBuffer.Get(), &uavDesc, m_backUAV.GetAddressOf()));

    // CS에서 사용할 Consts 버퍼
    D3D11Utils::CreateConstBuffer(m_device, m_constsCPU, m_constsGPU);

    // CS 만들기
    D3D11Utils::CreateComputeShader(m_device, L"Ex1401_CS.hlsl", m_testCS);

    // ComputeShader Test
    m_testComputePSO.m_computeShader = m_testCS;

    return true;
}

void Ex1401_Basic::Update(float dt) {
    D3D11Utils::UpdateBuffer(m_context, m_constsCPU, m_constsGPU);
}

void Ex1401_Basic::Render() {

    AppBase::SetPipelineState(m_testComputePSO);

    m_context->CSSetConstantBuffers(0, 1, m_constsGPU.GetAddressOf());
    m_context->CSSetUnorderedAccessViews(0, 1, m_backUAV.GetAddressOf(), NULL);

    // TODO: ThreadGroupCount를 쉐이더의 numthreads에 따라 잘 바꿔주기
    // TODO: ceil() 사용하는 이유 이해하기
    m_context->Dispatch(UINT(ceil(m_screenWidth / 256.0f)), m_screenHeight, 1);

    // 컴퓨터 쉐이더가 하던 일을 끝내게 만들고 Resources 해제
    AppBase::ComputeShaderBarrier();
}

void Ex1401_Basic::UpdateGUI() {
    ImGui::SliderFloat("Scale", &m_constsCPU.scale, 0.0f, 1.0f);
}

} // namespace hlab