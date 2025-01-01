#include "Samplers2.h"
#include "Engine.h"
#include "Device.h"
namespace dengine {
void Samplers::Init()
{
    D3D12_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    // s0, linearWarpSS
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
    m_sampDescs.push_back(sampDesc);
    // s1, linearClampSS
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    m_sampDescs.push_back(sampDesc);
    // s2, shadowPointSS
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.BorderColor[0] = 1.0f; // 큰 Z값
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    m_sampDescs.push_back(sampDesc);
    // s3, shadowCompareSS
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.BorderColor[0] = 100.0f; // 큰 Z값
    sampDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    m_sampDescs.push_back(sampDesc);
    // s4, pointWarpSS
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    m_sampDescs.push_back(sampDesc);
    // s5, linearMirrorSS
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
    m_sampDescs.push_back(sampDesc);
    // s6, pointClampSS
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    m_sampDescs.push_back(sampDesc);

    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = m_sampDescs.size();
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  // 셰이더에서 참조할 수 있도록 설정
    ThrowIfFailed(DEVICE->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_descHeap)));
     
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_descHeap->GetCPUDescriptorHandleForHeapStart());
    UINT descriptorSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    for (size_t i = 0; i < m_sampDescs.size(); ++i) {
        DEVICE->CreateSampler(&m_sampDescs[i], cpuHandle);
        cpuHandle.Offset(descriptorSize);  // 다음 샘플러의 주소로 이동
    }
}

}