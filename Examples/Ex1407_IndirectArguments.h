#pragma once

#include "AppBase.h"
#include "Model.h"
#include "StructuredBuffer.h"
#include "Texture2D.h"

namespace hlab {

class Ex1407_IndirectArguments : public AppBase {
  public:
    struct Particle {
        Vector3 position;
        Vector3 color;
    };

    Ex1407_IndirectArguments();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void DissipateDensity();
    void AdvectParticles();
    void DrawSprites();

  protected:
    // DrawInstanced(UINT 4개 필요)
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawinstanced
    struct IndirectArgs {
        UINT VertexCountPerInstance;
        UINT InstanceCount;
        UINT StartVertexLocation;
        UINT StartInstanceLocation;
    };

    vector<IndirectArgs> m_argsCPU = {
        {32, 1, 0, 0}, {64, 1, 0, 0}, {256, 1, 0, 0}};

    ComPtr<ID3D11Buffer> m_argsGPU;

    // Vertex Buffer (Compute Shader에서도 사용)
    StructuredBuffer<Particle> m_particles;

    // Shaders
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11ComputeShader> m_densitySourcingCS;
    ComPtr<ID3D11ComputeShader> m_densityDissipationCS;
    ComPtr<ID3D11GeometryShader> m_spriteGS;

    Texture2D m_densityTex;
};

} // namespace hlab