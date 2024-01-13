#pragma once

#include "GeometryGenerator.h"
#include "Model.h"

namespace hlab {

using std::make_shared;

class GrassModel : public Model {

  public:
    GrassModel() {} // Skip initialization
    GrassModel(ComPtr<ID3D11Device> &device,
               ComPtr<ID3D11DeviceContext> &context) {
        Initialize(device, context);
    }

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context) {

        // 잔디는 그림자맵 만들때 제외 (자체 그림자 효과 구현)
        m_castShadow = false;

        auto meshData = GeometryGenerator::MakeGrass();

        // Grass에서 사용하는 Vertex의 구조가 다른 Vertex와 다릅니다.
        vector<GrassVertex> grassVertices(meshData.vertices.size());
        for (int i = 0; i < grassVertices.size(); i++) {
            grassVertices[i].posModel = meshData.vertices[i].position;
            grassVertices[i].normalModel = meshData.vertices[i].normalModel;
            grassVertices[i].texcoord = meshData.vertices[i].texcoord;
        }

        // 여러가지 버퍼들 만들기
        D3D11Utils::CreateVertexBuffer(device, grassVertices, m_verticesGpu);

        assert(m_instancesCpu.size() > 0);

        m_instanceCount = UINT(m_instancesCpu.size());
        D3D11Utils::CreateInstanceBuffer(device, m_instancesCpu,
                                         m_instancesGpu);

        m_indexCount = UINT(meshData.indices.size());
        m_vertexCount = UINT(grassVertices.size());
        D3D11Utils::CreateIndexBuffer(device, meshData.indices, m_indexBuffer);

        m_meshConsts.GetCpu().world = Matrix();
        m_meshConsts.Initialize(device);
        m_materialConsts.Initialize(device);
    };

    void Render(ComPtr<ID3D11DeviceContext> &context) override {
        if (m_isVisible) {

            ID3D11Buffer *constBuffers[2] = {Model::m_meshConsts.Get(),
                                             Model::m_materialConsts.Get()};
            context->VSSetConstantBuffers(1, 2, constBuffers);
            context->PSSetConstantBuffers(1, 2, constBuffers);

            ID3D11Buffer *const vertexBuffers[2] = {m_verticesGpu.Get(),
                                                    m_instancesGpu.Get()};
            const UINT strides[2] = {sizeof(GrassVertex),
                                     sizeof(GrassInstance)};
            const UINT offsets[2] = {0, 0};
            context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
            context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                      0);
            context->IASetInputLayout(Graphics::grassIL.Get());
            context->DrawIndexedInstanced(m_indexCount, m_instanceCount, 0, 0,
                                          0);
        }
    };

    GraphicsPSO &GetPSO(const bool wired) override {
        return wired ? Graphics::grassWirePSO : Graphics::grassSolidPSO;
    }

    GraphicsPSO &GetReflectPSO(const bool wired) override {
        return wired ? Graphics::grassWirePSO
                     : Graphics::grassSolidPSO; // Dummy (미구현)
    }

    GraphicsPSO &GetDepthOnlyPSO() override {
        return Graphics::depthOnlyPSO; // Dummy (미구현)
    }

  public:
    vector<GrassInstance> m_instancesCpu;

    ComPtr<ID3D11Buffer> m_verticesGpu;
    ComPtr<ID3D11Buffer> m_instancesGpu;
    ComPtr<ID3D11Buffer> m_indexBuffer;

    UINT m_indexCount = 0;
    UINT m_vertexCount = 0;
    UINT m_offset = 0;
    UINT m_instanceCount = 0;
};

} // namespace hlab