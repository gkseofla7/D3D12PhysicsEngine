#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1502_SpriteFireEffect : public AppBase {
  public:
    struct Particle {
        Vector3 position;
        Vector3 velocity;
        Vector3 color;
        float life = 0.0f;
        float radius = 1.0f;
    };

    Ex1502_SpriteFireEffect();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void DrawSprites();

  protected:
    vector<Particle> m_particlesCPU;

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

    // Sprite
    ComPtr<ID3D11Texture2D> m_spriteTex;
    ComPtr<ID3D11ShaderResourceView> m_spriteSRV;
};

} // namespace hlab