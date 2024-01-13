#pragma once

#include "D3D11Utils.h"
#include <assert.h>
#include <iostream>
#include <vector>

namespace hlab {
using std::vector;

template <typename T_ELEMENT> class StructuredBuffer {
  public:
    virtual void Initialize(ComPtr<ID3D11Device> &device,
                            const UINT numElements) {
        m_cpu.resize(numElements);
        Initialize(device);
    }

    virtual void Initialize(ComPtr<ID3D11Device> &device) {
        D3D11Utils::CreateStructuredBuffer(device, UINT(m_cpu.size()),
                                           sizeof(T_ELEMENT), m_cpu.data(),
                                           m_gpu, m_srv, m_uav);
        // Staging은 주로 디버깅 용도입니다.
        D3D11Utils::CreateStagingBuffer(device, UINT(m_cpu.size()),
                                        sizeof(T_ELEMENT), NULL, m_staging);
    }

    void Upload(ComPtr<ID3D11DeviceContext> &context) {
        Upload(context, m_cpu);
    }

    void Upload(ComPtr<ID3D11DeviceContext> &context,
                vector<T_ELEMENT> &arrCpu) {

        assert(arrCpu.size() == m_cpu.size());

        D3D11Utils::CopyToStagingBuffer(context, m_staging,
                                        UINT(arrCpu.size() * sizeof(T_ELEMENT)),
                                        arrCpu.data());
        context->CopyResource(m_gpu.Get(), m_staging.Get());
    }

    void Download(ComPtr<ID3D11DeviceContext> &context) {
        Download(context, m_cpu);
    }

    void Download(ComPtr<ID3D11DeviceContext> &context,
                  vector<T_ELEMENT> &arrCpu) {

        assert(arrCpu.size() == m_cpu.size());

        context->CopyResource(m_staging.Get(), m_gpu.Get());
        D3D11Utils::CopyFromStagingBuffer(
            context, m_staging, UINT(arrCpu.size() * sizeof(T_ELEMENT)),
            arrCpu.data());
    }

    const auto GetBuffer() { return m_gpu.Get(); }
    const auto GetSRV() { return m_srv.Get(); }
    const auto GetUAV() { return m_uav.Get(); }
    const auto GetAddressOfSRV() { return m_srv.GetAddressOf(); }
    const auto GetAddressOfUAV() { return m_uav.GetAddressOf(); }
    const auto GetStaging() { return m_staging.Get(); }

    vector<T_ELEMENT> m_cpu;
    ComPtr<ID3D11Buffer> m_gpu;
    ComPtr<ID3D11Buffer> m_staging;

    ComPtr<ID3D11ShaderResourceView> m_srv;
    ComPtr<ID3D11UnorderedAccessView> m_uav;
};

// StructuredBuffer 대신 AppendBuffer 사용할 수도 있음
template <typename T_ELEMENT>
class AppendBuffer : public StructuredBuffer<T_ELEMENT> {

    typedef StructuredBuffer<T_ELEMENT> BASE;

  public:
    void Initialize(ComPtr<ID3D11Device> &device) {
        D3D11Utils::CreateAppendBuffer(device, UINT(BASE::m_cpu.size()),
                                       sizeof(T_ELEMENT), BASE::m_cpu.data(),
                                       BASE::m_gpu, BASE::m_srv, BASE::m_uav);
    }

    friend void swap(AppendBuffer<T_ELEMENT> &lhs,
                     AppendBuffer<T_ELEMENT> &rhs) {

        // std::cout << "custom swap \n";

        std::swap(lhs.m_cpu, rhs.m_cpu);
        std::swap(lhs.m_gpu, rhs.m_gpu);
        std::swap(lhs.m_srv, rhs.m_srv);
        std::swap(lhs.m_uav, rhs.m_uav);
    }
};

} // namespace hlab
