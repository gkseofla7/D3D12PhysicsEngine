#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "Model.h"

namespace hlab {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

struct BillboardConsts {
    float widthWorld;
    Vector3 directionWorld;
};

static_assert((sizeof(BillboardConsts) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class BillboardModel : public Model {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<Vector4> &points, const float width,
                    const ComPtr<ID3D11PixelShader> &pixelShader);

    void Render(ComPtr<ID3D11DeviceContext> &context) override;
    void RenderNormals(ComPtr<ID3D11DeviceContext> &context) override{};
    void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context) override{};
    void
    RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context) override{};

    GraphicsPSO &GetPSO(const bool wired) override {
        // return wired ? Graphics::defaultWirePSO : fireballPSO;
        return Graphics::defaultWirePSO;
    }
    virtual bool IsPostProcess() { return false; }
  public:
    ConstantBuffer<BillboardConsts> m_billboardConsts;

  protected:
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11GeometryShader> m_geometryShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    uint32_t m_indexCount = 0;
};

} // namespace hlab