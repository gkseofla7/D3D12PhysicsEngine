#include "BillboardModel2.h"
#include "D3D12Core/D3D12Utils.h"
#include "GameCore/MeshLoadHelper2.h"
#include <numeric>

namespace dengine {

void BillboardModel::Initialize(const std::vector<Vector4> &points,
                                const float width) 
{    
    BillboardModel::m_castShadow = false;
   // m_meshKey = //MeshLoadHelper::LoadSquareMesh(width);
}


void BillboardModel::Render() 
{
    //if (m_isVisible) {
    //    // 편의상 PSO 설정을 Render()에서 바꾸는 방식
    //    context->IASetInputLayout(m_inputLayout.Get());
    //    context->VSSetShader(m_vertexShader.Get(), 0, 0);
    //    context->PSSetShader(m_pixelShader.Get(), 0, 0);
    //    ID3D11Buffer *constBuffers[2] = {this->m_meshConsts.Get(),
    //                                     this->m_materialConsts.Get()};
    //    context->VSSetConstantBuffers(1, 2, constBuffers);
    //    context->VSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
    //    context->PSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
    //    context->GSSetConstantBuffers(3, 1, m_billboardConsts.GetAddressOf());
    //    context->GSSetShader(m_geometryShader.Get(), 0, 0);
    //    context->RSSetState(Graphics::solidBothRS.Get());
    //    context->OMSetBlendState(Graphics::alphaBS.Get(),
    //                             Graphics::defaultSolidPSO.m_blendFactor,
    //                             0xffffffff);
    //    UINT stride = sizeof(Vector4); // sizeof(Vertex);
    //    UINT offset = 0;
    //    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),
    //                                &stride, &offset);
    //    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    //    context->Draw(m_indexCount, 0);
    //    context->GSSetShader(NULL, 0, 0);
    //}
}

} // namespace hlab