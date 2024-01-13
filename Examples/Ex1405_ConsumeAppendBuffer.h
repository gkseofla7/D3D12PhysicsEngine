#pragma once

#include "AppBase.h"
#include "Buffers.h"
#include "Model.h"
#include "StructuredBuffer.h"

namespace hlab {

class Ex1405_ConsumeAppendBuffer : public AppBase {
  public:
    struct Particle {
        Vector3 position;
        Vector3 color;
    };

    Ex1405_ConsumeAppendBuffer();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    AppendBuffer<Particle> m_consume;
    AppendBuffer<Particle> m_append;

    StagingBuffer<uint32_t> m_countStaging;

    // Shaders
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11ComputeShader> m_computeShader;
};

} // namespace hlab