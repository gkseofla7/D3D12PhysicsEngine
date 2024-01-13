#pragma once

#include "AppBase.h"
#include "Model.h"
#include "SphSimulation.h"

namespace hlab {

class Ex1503_SphWater : public AppBase {
  public:
    // SphSimulation::Particle 사용

    Ex1503_SphWater();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void DrawSprites();

  protected:
    SphSimulation m_sph;

    // vector<Particle> m_particlesCPU; // m_sph.m_particles 사용

    // Vertex Buffer (Compute Shader에서도 사용)
    ComPtr<ID3D11Buffer> m_particlesGPU;
    ComPtr<ID3D11Buffer> m_particlesStagingGPU;

    // Shaders
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11GeometryShader> m_spriteGS;

    ComPtr<ID3D11ShaderResourceView> m_particlesSRV;
    ComPtr<ID3D11UnorderedAccessView> m_particlesUAV;
};

} // namespace hlab