#pragma once
#include "EnginePch.h"
#include "Engine.h"
#include "../ThreadPool.h"
#include <shared_mutex>
// AppBase�� ExampleApp�� �����ϱ� ����
// �ݺ��ؼ� ���Ǵ� ���̴� ����, ���� ���� ���� �и�
// Parameter�� ������ �� const�� �տ� �δ� ���� �Ϲ���������
// device�� ���ƻ��� �߿伺 ������ ���ܷ� �� �տ� �׽��ϴ�.
// ���ǰ� ����Ǹ鼭 ���ݾ� ����� �߰��˴ϴ�.

namespace dengine {

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
class Resource;

struct ResourceInfo
{
    ELoadType loadType = ELoadType::NotLoaded;
    ComPtr<ID3D12Resource> resource;
    vector<shared_ptr<Resource>> pendingResources;


    std::shared_mutex resMutex;
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
    // ShaderMacros ����� �� ����
    // {D3D_SHADER_MACRO("SKINNED", "1"), D3D_SHADER_MACRO(NULL, NULL)};
    // �� �ڿ� NULL, NULL �ʼ�

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
        vertexBufferView.StrideInBytes = sizeof(T_VERTEX); // ���� 1�� ũ��
        vertexBufferView.SizeInBytes = bufferSize; // ������ ũ��	
    }



    // Texture
    static void LoadTexture(const std::wstring path, const bool usSRGB, bool bAsync,
        shared_ptr<Resource> outResource);

    static void CreateTexture(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty,
        D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor, shared_ptr<Resource> outResource);


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
    static void LoadTextureImpl(const std::wstring path, const bool usSRGB);
    static void LoadTextureNotUsingScratchImage(const std::wstring path, const bool usSRGB);
private:
    static std::unordered_map<std::string, ImageInfo> imageMap;
    static std::unordered_map<std::wstring, ResourceInfo> s_resourceMap;

};
} // namespace dengine
