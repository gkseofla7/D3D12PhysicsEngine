#pragma once

#include "AppBase.h"
#include "BitonicSort.h"
#include "Buffers.h"
#include "Model.h"
#include "StructuredBuffer.h"
#include "Texture2D.h"
#include "Texture3D.h"

namespace hlab {

class Ex1606_HybridWater : public AppBase {
  public:
    __declspec(align(256)) struct FluidConsts {
        Vector3 dxBase = Vector3(1.0f);
        float dt = 1.0f / 60.0f;
        Vector3 dxUp = Vector3(1.0f);
        float time = 0.0f;
        int upScale = 1;
        int numNewParticles = 0;
        float turbulence = 0.8f;
        float sourceStrength = 2.2f;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
    };

    struct Particle {
        Vector3 pos;
        Vector3 vel;
    };

    Ex1606_HybridWater();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void Projection();
    void ParticleStep();

  protected:
    shared_ptr<Model> m_volumeModel;

    Texture3D m_velocity, m_velocityTemp;
    Texture3D m_pressure, m_pressureTemp;
    Texture3D m_divergence;
    Texture3D m_density;
    Texture3D m_bc; // Boundary conditions
    Texture3D m_noise;
    Texture3D m_firstIndex;
    Texture3D m_signedDistance;

    ComPtr<ID3D11ComputeShader> m_applyPressureCS;
    ComPtr<ID3D11ComputeShader> m_divergenceCS;
    ComPtr<ID3D11ComputeShader> m_jacobiCS;
    ComPtr<ID3D11ComputeShader> m_diffUpSampleCS;
    ComPtr<ID3D11ComputeShader> m_particleStepCS;
    ComPtr<ID3D11ComputeShader> m_firstIndexCS;
    ComPtr<ID3D11ComputeShader> m_particleToGridCS;

    ComPtr<ID3D11PixelShader> m_signedDistancePS;

    ConstantBuffer<FluidConsts> m_fluidConsts;
    ConstantBuffer<VolumeConsts> m_volumeConsts;

    // Particles

    StructuredBuffer<Particle> m_particles;
    StructuredBuffer<uint32_t> m_numActiveParticles;
    BitonicSort m_sort;

    UINT m_width = 0;
    UINT m_height = 0;
    UINT m_depth = 0;
    UINT m_widthUp = 0;
    UINT m_heightUp = 0;
    UINT m_depthUp = 0;

    int m_upScale = 1;
    bool m_renderParticles = false;
    bool m_renderDensity = false;
    bool m_renderMarchingCubes = false;
    bool m_renderWired = false;
    bool m_renderRayCasting = true;

    // Particle rendering

    ComPtr<ID3D11VertexShader> m_particleVS;
    ComPtr<ID3D11PixelShader> m_particlePS;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    ComPtr<ID3D11Buffer> m_dummyVertexBuffer;

    // Marching cubes
    Texture2D m_triTable;
    ComPtr<ID3D11VertexShader> m_mcVS;
    ComPtr<ID3D11GeometryShader> m_mcGS;
};

} // namespace hlab