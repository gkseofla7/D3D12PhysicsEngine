#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1401_Basic : public AppBase {
  public:
    __declspec(align(256)) struct Constants {
        float scale = 1.0f;
    };

    Ex1401_Basic();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    // 예시용이라서 GraphicsCommon.h/cpp에 구현하지 않음
    ComPtr<ID3D11ComputeShader> m_testCS;
    ComputePSO m_testComputePSO;

    ComPtr<ID3D11UnorderedAccessView> m_backUAV;

    Constants m_constsCPU;
    ComPtr<ID3D11Buffer> m_constsGPU;
};

} // namespace hlab