#pragma once
#include "EnginePch.h"
#include "Texture.h"
namespace hlab {

class Texture;
struct DMesh {
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    
    D3D12_VERTEX_BUFFER_VIEW	vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW	indexBufferView;

    Texture albedoTexture;
    Texture emissiveTexture;
    Texture normalTexture;
    Texture heightTexture;
    Texture aoTexture;
    Texture metallicRoughnessTexture;


    UINT indexCount = 0;
    UINT vertexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};

} // namespace hlab
