﻿#pragma once
#include "D3D12Core/EnginePch.h"
#include "D3D12Core/Texture.h"
namespace dengine {

class Texture;
struct DMesh {
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    
    D3D12_VERTEX_BUFFER_VIEW	vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW	indexBufferView;

    shared_ptr<Texture> albedoTexture;
    shared_ptr<Texture> emissiveTexture;
    shared_ptr<Texture> normalTexture;
    shared_ptr<Texture> heightTexture;
    shared_ptr<Texture> aoTexture;
    shared_ptr<Texture> metallicRoughnessTexture;


    UINT indexCount = 0;
    UINT vertexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};

} // namespace dengine
