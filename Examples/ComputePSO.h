#pragma once

#include "D3D11Utils.h"

namespace hlab {

// 참고: DirectX_Graphic-Samples 미니엔진
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/PipelineState.h

// GraphicsPSO와 비교했을 때 훨씬 단순합니다.
class ComputePSO {
  public:
    void operator=(const ComputePSO &pso) {
        m_computeShader = pso.m_computeShader;
    };

  public:
    ComPtr<ID3D11ComputeShader> m_computeShader;
};

} // namespace hlab