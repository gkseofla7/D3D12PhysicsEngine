#pragma once
#include "EnginePch.h"
#include "Engine.h"
#include "GameCore/ThreadPool.h"
#include "CommandQueue.h"
#include <shared_mutex>
#include <mutex>
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

    std::mutex imageMutex;
};
class Resource;

struct ResourceInfo
{
    ELoadType loadType = ELoadType::NotLoaded;
    ComPtr<ID3D12Resource> resource;
    vector<shared_ptr<Resource>> awaitingSetResources;


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
    // ShaderMacros 사용할 때 예시
    // {D3D_SHADER_MACRO("SKINNED", "1"), D3D_SHADER_MACRO(NULL, NULL)};
    // 맨 뒤에 NULL, NULL 필수

    static void CreateIndexBuffer(ComPtr<ID3D12Device> device,
        const vector<uint32_t>& indices,
        ComPtr<ID3D12Resource>& indexBuffer,
        D3D12_INDEX_BUFFER_VIEW& indexBufferView);

    template <typename T_VERTEX>
    static void CreateVertexBuffer(
        ComPtr<ID3D12Device> device,
        const vector<T_VERTEX>& vertices,
        ComPtr<ID3D12Resource>& vertexBuffer,
        D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
    {
        uint32 bufferSize = static_cast<uint32>(vertices.size() * sizeof(T_VERTEX));

        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer));
        ResourceCommandList rscCommandList = RESOURCE_CMD_LIST;
        CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            vertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST);
        rscCommandList.m_resCmdList->ResourceBarrier(1, &toCopyDest);

        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        ComPtr<ID3D12Resource> vertexUploadBuffer;
        device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexUploadBuffer));

        void* mappedData = nullptr;
        CD3DX12_RANGE readRange(0, 0); // 읽지 않을 거라서
        vertexUploadBuffer->Map(0, &readRange, &mappedData);
        memcpy(mappedData, vertices.data(), bufferSize);
        vertexUploadBuffer->Unmap(0, nullptr);
        rscCommandList.m_resCmdList->CopyBufferRegion(vertexBuffer.Get(), 0, vertexUploadBuffer.Get(), 0, bufferSize);

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            vertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        rscCommandList.m_resCmdList->ResourceBarrier(1, &barrier);

        // 6. Vertex Buffer View 설정
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(T_VERTEX);
        vertexBufferView.SizeInBytes = bufferSize;
        GEngine->GetResourceCmdQueue()->FlushResourceCommandQueue(rscCommandList, true);
    }

    // Texture
    static void LoadTexture(const std::wstring path, const bool usSRGB, bool bAsync,
        shared_ptr<Resource> outResource);
    static void LoadAlbedoOpacityTexture(ComPtr<ID3D12Device> device, const std::string albedoFilename,
        const std::string opacityFilename, const bool usSRGB, ComPtr<ID3D12Resource>& texture);
    static void LoadMetallicRoughnessTexture(
        ComPtr<ID3D12Device> device, const std::string metallicFiilename,
        const std::string roughnessFilename, ComPtr<ID3D12Resource>& texture);
    static void CreateTexture(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty,
        D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor, shared_ptr<Resource> outResource);
    static void CreateDDSTexture(ComPtr<ID3D12Device> &device, ComPtr<ID3D12CommandQueue> m_commandQueue,
                                 const wstring&&filename, const bool isCubeMap);

    static size_t GetPixelSize(DXGI_FORMAT pixelFormat);

private:
    static void LoadTextureImpl(const std::wstring path, const bool usSRGB);
    static void LoadTextureNotUsingScratchImage(const std::wstring path, const bool usSRGB);

    static void CreateTextureFromImage(ComPtr<ID3D12Device>& device,
        const int width, const int height, const vector<uint8_t>& image,
        const DXGI_FORMAT pixelFormat, ComPtr<ID3D12Resource>& texture);
private:
    static std::mutex s_imageMapMutex;
    static std::unordered_map<std::string, std::unique_ptr<ImageInfo>> imageMap;
    static std::unordered_map<std::wstring, ResourceInfo> s_resourceMap;

};
} // namespace dengine
