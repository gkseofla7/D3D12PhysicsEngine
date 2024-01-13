#pragma once

#include "D3D11Utils.h"
#include "StructuredBuffer.h"
#include "Timer.h"

namespace hlab {

class BitonicSort {
  public:
    struct Element {
        uint32_t key;
        uint32_t value;
    };

    __declspec(align(256)) struct Consts {
        uint32_t k;
        uint32_t j;
    };

    BitonicSort(){};

    BitonicSort(ComPtr<ID3D11Device> &device, const UINT numElements,
                const wstring shaderFilename) {
        Initialize(device, numElements, shaderFilename);
    };

    void Initialize(ComPtr<ID3D11Device> &device, const UINT numElements,
                    const wstring shaderFilename);

    void SortGPU(ComPtr<ID3D11Device> &device,
                 ComPtr<ID3D11DeviceContext> &context);

    static void SortCPU(vector<Element> &arr);
    static void TestBitonicSort(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context);

    StructuredBuffer<Element> m_array;

  protected:
    vector<Consts> m_constsCpu;
    vector<ComPtr<ID3D11Buffer>> m_constsGpu;

    ComPtr<ID3D11ComputeShader> m_bitonicSortCS;

    UINT m_numElements = 0;
};

} // namespace hlab