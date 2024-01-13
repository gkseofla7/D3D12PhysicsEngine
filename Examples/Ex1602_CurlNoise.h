#pragma once

#include "AppBase.h"
#include "Model.h"
#include "Texture2D.h"

namespace hlab {

class Ex1602_CurlNoise : public AppBase {
  public:
    struct Particle {
        Vector3 position;
        Vector3 color;
    };

    Ex1602_CurlNoise();

    virtual bool Initialize() override;
    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    ComPtr<ID3D11ComputeShader> m_curlNoiseCS;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11GeometryShader> m_spriteGS;
    ComPtr<ID3D11ComputeShader> m_densityDissipationCS;

    Texture2D m_density;

    vector<Particle> m_particlesCPU;
    ComPtr<ID3D11Buffer> m_particlesGPU;
    ComPtr<ID3D11ShaderResourceView> m_particlesSRV;
    ComPtr<ID3D11UnorderedAccessView> m_particlesUAV;
};

} // namespace hlab