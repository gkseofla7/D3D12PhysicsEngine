#pragma once

#include <directxtk/SimpleMath.h>

#include "GraphicsCommon.h"
#include "Texture2D.h"

namespace hlab {

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector4;

// "Stable Fluids" by Jos Stam
class StableFluids {
  public:
    __declspec(align(256)) struct Consts {
        float dt;
        float viscosity;
        Vector2 sourcingVelocity;
        Vector4 sourcingDensity;
        uint32_t i;
        uint32_t j;
    };

    void Initialize(ComPtr<ID3D11Device> &device, const UINT width,
                    const UINT height);
    void Update(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context, float dt);
    void ComputeShaderBarrier(ComPtr<ID3D11DeviceContext> &context);
    void Sourcing(ComPtr<ID3D11DeviceContext> &context);
    void Diffuse(ComPtr<ID3D11DeviceContext> &context);
    void Projection(ComPtr<ID3D11DeviceContext> &context);
    void Advection(ComPtr<ID3D11DeviceContext> &context);

    Texture2D m_velocity, m_velocityTemp;
    Texture2D m_vorticity;
    Texture2D m_pressure, m_pressureTemp;
    Texture2D m_divergence;
    Texture2D m_density, m_densityTemp;

    Consts m_constsCPU;

  private:
    ComPtr<ID3D11ComputeShader> m_advectionCS;
    ComPtr<ID3D11ComputeShader> m_applyPressureCS;
    ComPtr<ID3D11ComputeShader> m_diffuseCS;
    ComPtr<ID3D11ComputeShader> m_divergenceCS;
    ComPtr<ID3D11ComputeShader> m_jacobiCS;
    ComPtr<ID3D11ComputeShader> m_sourcingCS;
    ComPtr<ID3D11ComputeShader> m_computeVorticityCS;
    ComPtr<ID3D11ComputeShader> m_confineVorticityCS;

    ComPtr<ID3D11Buffer> m_constsGPU;

    UINT m_width = 0;
    UINT m_height = 0;
};
} // namespace hlab