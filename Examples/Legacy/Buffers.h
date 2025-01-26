#pragma once

#include "D3D11Utils.h"
#include <vector>

namespace hlab {

using std::vector;

template <typename T> class StagingBuffer {
  public:
    void Initialize(ComPtr<ID3D11Device> &device, const vector<T> &data) {
        m_cpu = data;
        D3D11Utils::CreateStagingBuffer(device, UINT(data.size()), sizeof(T),
                                        data.data(), m_gpu);
    }

    void Download(ComPtr<ID3D11DeviceContext> &context) {
        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(m_gpu.Get(), NULL, D3D11_MAP_READ, NULL, &ms);
        uint8_t *pData = (uint8_t *)ms.pData;
        memcpy(m_cpu.data(), &pData[0], sizeof(T) * m_cpu.size());
        context->Unmap(m_gpu.Get(), NULL);
    }

    vector<T> m_cpu;
    ComPtr<ID3D11Buffer> m_gpu;
};

} // namespace hlab