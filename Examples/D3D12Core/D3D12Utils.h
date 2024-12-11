#pragma once
#include "EnginePch.h"
#include "Engine.h"
#include "../ThreadPool.h"
// AppBase와 ExampleApp을 정리하기 위해
// 반복해서 사용되는 쉐이더 생성, 버퍼 생성 등을 분리
// Parameter를 나열할 때 const를 앞에 두는 것이 일반적이지만
// device는 문맥상의 중요성 때문에 예외로 맨 앞에 뒀습니다.
// 강의가 진행되면서 조금씩 기능이 추가됩니다.

namespace hlab {

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

struct ImageInfo
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> image;
    DXGI_FORMAT pixelFormat;
};


class D3D12Utils {
  public:
      static void CreateVertexShader(
          ComPtr<ID3D12Device> device, wstring filename,
          ComPtr<ID3DBlob>& vertexShader,
          const vector<D3D_SHADER_MACRO> shaderMacros = {/* Empty default */ });
      static void CreatePixelShader(ComPtr<ID3D12Device> device,
          const wstring& filename,
          ComPtr<ID3DBlob>& pixelShader);
      static void CreatePipelineState(ComPtr<ID3D12Device>& device, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, ComPtr<ID3D12PipelineState>& OutPipelineState);
    // ShaderMacros 사용할 때 예시
    // {D3D_SHADER_MACRO("SKINNED", "1"), D3D_SHADER_MACRO(NULL, NULL)};
    // 맨 뒤에 NULL, NULL 필수

    static void CreateIndexBuffer(ComPtr<ID3D12Device> device,
        const vector<uint32_t>& indices,
        ComPtr<ID3D12Resource>& indexBuffer,
        D3D12_INDEX_BUFFER_VIEW& indexBufferView);


    template <typename T_VERTEX>
    static void CreateVertexBuffer(ComPtr<ID3D12Device> device,
        const vector<T_VERTEX>& vertices,
        ComPtr<ID3D12Resource>&	vertexBuffer,
        D3D12_VERTEX_BUFFER_VIEW& vertexBufferView) {
        uint32 bufferSize = vertices.size() * sizeof(T_VERTEX);

        D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        DEVICE->CreateCommittedResource(
            &heapProperty,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer));

        // Copy the triangle data to the vertex buffer.
        void* vertexDataBuffer = nullptr;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        vertexBuffer->Map(0, &readRange, &vertexDataBuffer);
        ::memcpy(vertexDataBuffer, &vertices[0], bufferSize);
        vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(T_VERTEX); // 정점 1개 크기
        vertexBufferView.SizeInBytes = bufferSize; // 버퍼의 크기	
    }



    // Texture
    static void CreateTexture(ComPtr<ID3D12Device> device,
                  const std::string filename, const bool usSRGB,
                    ComPtr<ID3D12Resource>&texture);

    static void CreateTexture(ComPtr<ID3D12Device> device, const std::string albedoFilename,
        const std::string opacityFilename,const bool usSRGB, ComPtr<ID3D12Resource>& texture);
  
    static void CreateMetallicRoughnessTexture(
        ComPtr<ID3D12Device> device, const std::string metallicFiilename,
        const std::string roughnessFilename, ComPtr<ID3D12Resource>& texture);


    static void CreateTextureHelper(ComPtr<ID3D12Device>& device,
        const int width, const int height, const vector<uint8_t>& image,
        const DXGI_FORMAT pixelFormat, ComPtr<ID3D12Resource>& texture);


    static void CreateDDSTexture(ComPtr<ID3D12Device> &device, ComPtr<ID3D12CommandQueue> m_commandQueue,
                                 const wstring&&filename, const bool isCubeMap);


    static size_t GetPixelSize(DXGI_FORMAT pixelFormat);
private:
    static std::unordered_map<std::string, ImageInfo> imageMap;
};
} // namespace hlab
