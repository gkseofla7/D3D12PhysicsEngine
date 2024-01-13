#include "BillboardModel.h"

#include <numeric>

namespace hlab {

void BillboardModel::Initialize(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                const std::vector<Vector4> &points,
                                const float width,
                                const ComPtr<ID3D11PixelShader> &pixelShader) {

    BillboardModel::m_castShadow = false;

    D3D11Utils::CreateVertexBuffer(device, points, m_vertexBuffer);

    m_indexCount = uint32_t(points.size());
    m_vertexShader = Graphics::billboardVS;
    m_geometryShader = Graphics::billboardGS;
    m_inputLayout = Graphics::billboardIL;
    m_pixelShader = pixelShader;

    m_billboardConsts.GetCpu().widthWorld = width;
    m_billboardConsts.Initialize(device);
    m_meshConsts.GetCpu().world = Matrix();
    m_meshConsts.Initialize(device);
    m_materialConsts.Initialize(device);
}

void BillboardModel::Render(ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        // 편의상 PSO 설정을 Render()에서 바꾸는 방식
        context->IASetInputLayout(m_inputLayout.Get());
        context->VSSetShader(m_vertexShader.Get(), 0, 0);
        context->PSSetShader(m_pixelShader.Get(), 0, 0);
        ID3D11Buffer *constBuffers[2] = {this->m_meshConsts.Get(),
                                         this->m_materialConsts.Get()};
        context->VSSetConstantBuffers(1, 2, constBuffers);
        context->VSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
        context->PSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
        context->GSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
        context->GSSetShader(m_geometryShader.Get(), 0, 0);
        context->RSSetState(Graphics::solidBothRS.Get());
        context->OMSetBlendState(Graphics::alphaBS.Get(),
                                 Graphics::defaultSolidPSO.m_blendFactor,
                                 0xffffffff);
        UINT stride = sizeof(Vector4); // sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        context->Draw(m_indexCount, 0);
        context->GSSetShader(NULL, 0, 0);
    }
}

} // namespace hlab