#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1603_Cloud : public AppBase {
  public:
    Ex1603_Cloud();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    int m_volumeWidth = 128;
    int m_volumeHeight = 128;
    int m_volumeDepth = 128;
    int m_lightWidth = 128 / 4; // 라이트맵은 낮은 해상도
    int m_lightHeight = 128 / 4;
    int m_lightDepth = 128 / 4;

    shared_ptr<Model> m_volumeModel;

    ComPtr<ID3D11ComputeShader> m_cloudDensityCS;
    ComPtr<ID3D11ComputeShader> m_cloudLightingCS;

    VolumeConsts m_volumeConstsCpu;
    ComPtr<ID3D11Buffer> m_volumeConstsGpu;
};

} // namespace hlab