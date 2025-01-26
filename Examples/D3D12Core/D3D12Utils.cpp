// #define _CRT_SECURE_NO_WARNINGS // stb_image_write compile error fix
#include "D3D12Utils.h"
//#include "Engine.h"
#include "CommandQueue.h"
#include "Device.h"
#include "Resource.h"

#include <DirectXTexEXR.h> // EXR 형식 HDRI 읽기
#include <algorithm>
#include <cctype>
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <execution>
#include <fp16.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>

//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace dengine {
using namespace std;
using namespace DirectX;
std::mutex D3D12Utils::s_imageMapMutex;
unordered_map<string, std::unique_ptr<ImageInfo>> D3D12Utils::imageMap;
unordered_map<std::wstring, ResourceInfo> D3D12Utils::s_resourceMap;
void CheckResult(HRESULT hr, ID3DBlob* errorBlob) {
    if (FAILED(hr)) {
        // 파일이 없을 경우
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            cout << "File not found." << endl;
        }

        // 에러 메시지가 있으면 출력
        if (errorBlob) {
            cout << "Shader compile error\n"
                << (char*)errorBlob->GetBufferPointer() << endl;
        }
    }
}
std::string wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}
void D3D12Utils::CreateVertexShader(
    ComPtr<ID3D12Device> device, wstring filename,
    ComPtr<ID3DBlob>& vertexShader,
    const vector<D3D_SHADER_MACRO> shaderMacros)
{

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &vertexShader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}

void D3D12Utils::CreatePixelShader(ComPtr<ID3D12Device> device,
    const wstring& filename,
    ComPtr<ID3DBlob>& pixelShader)
{
    ComPtr<ID3DBlob> errorBlob;
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

    CheckResult(hr, errorBlob.Get());
}

void D3D12Utils::CreatePipelineState(ComPtr<ID3D12Device>& device, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, ComPtr<ID3D12PipelineState>& OutPipelineState)
{
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&OutPipelineState)));
}


void D3D12Utils::CreateIndexBuffer(ComPtr<ID3D12Device> device,
    const std::vector<uint32_t>& indices,
    ComPtr<ID3D12Resource>& indexBuffer,
    D3D12_INDEX_BUFFER_VIEW& indexBufferView) {
    uint32 indexCount = static_cast<uint32>(indices.size());
    uint32 bufferSize = indexCount * sizeof(uint32);

    D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProperty,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBuffer));

    void* indexDataBuffer = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    indexBuffer->Map(0, &readRange, &indexDataBuffer);
    ::memcpy(indexDataBuffer, &indices[0], bufferSize);
    indexBuffer->Unmap(0, nullptr);

    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = bufferSize;

}


void ReadEXRImage(const std::string filename, std::vector<uint8_t>& image,
    int& width, int& height, DXGI_FORMAT& pixelFormat) {

    const std::wstring wFilename(filename.begin(), filename.end());

    TexMetadata metadata;
    ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metadata));

    ScratchImage scratchImage;
    ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));

    width = static_cast<int>(metadata.width);
    height = static_cast<int>(metadata.height);
    pixelFormat = metadata.format;

    cout << filename << " " << metadata.width << " " << metadata.height
        << metadata.format << endl;

    image.resize(scratchImage.GetPixelsSize());
    memcpy(image.data(), scratchImage.GetPixels(), image.size());

    // 데이터 범위 확인해보기
    vector<float> f32(image.size() / 2);
    uint16_t* f16 = (uint16_t*)image.data();
    for (int i = 0; i < image.size() / 2; i++) {
        f32[i] = fp16_ieee_to_fp32_value(f16[i]);
    }

    const float minValue = *std::min_element(f32.begin(), f32.end());
    const float maxValue = *std::max_element(f32.begin(), f32.end());

    cout << minValue << " " << maxValue << endl;

    // f16 = (uint16_t *)image.data();
    // for (int i = 0; i < image.size() / 2; i++) {
    //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
    // }
}

void ReadImage(const std::string filename, std::vector<uint8_t>& image,
    int& width, int& height) {
    int channels;

    unsigned char* img =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);

    // assert(channels == 4);

    cout << "ReadImage() " << filename << " " << width << " " << height << " "
        << channels << endl;

    // 4채널로 만들어서 복사
    image.resize(width * height * 4);

    if (channels == 1) {
        for (size_t i = 0; i < width * height; i++) {
            uint8_t g = img[i * channels + 0];
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = g;
            }
        }
    }
    else if (channels == 2) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 2; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 3) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 3; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 4) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    }
    else {
        std::cout << "Cannot read " << channels << " channels" << endl;
    }

    delete[] img;
}

void ReadImage(const std::string albedoFilename,
    const std::string opacityFilename, std::vector<uint8_t>& image,
    int& width, int& height) {

    ReadImage(albedoFilename, image, width, height);

    std::vector<uint8_t> opacityImage;
    int opaWidth, opaHeight;

    ReadImage(opacityFilename, opacityImage, opaWidth, opaHeight);

    assert(width == opaWidth && height == opaHeight);

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            image[3 + 4 * i + 4 * width * j] =
                opacityImage[4 * i + 4 * width * j]; // Copy alpha channel
        }
}

void D3D12Utils::CreateTextureHelper(ComPtr<ID3D12Device>& device,
    const int width, const int height, const vector<uint8_t>& image,
    const DXGI_FORMAT pixelFormat, ComPtr<ID3D12Resource>& texture)
{
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 0;// Check. 1?
    textureDesc.DepthOrArraySize = 1;
    textureDesc.Format = pixelFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.SampleDesc.Quality = 1;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)));

    // 스테이징 텍스춰 만들고 CPU에서 이미지를 복사
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    CD3DX12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ComPtr<ID3D12Resource> textureUploadHeap;
    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)));
    const int texturePixelSize = GetPixelSize(pixelFormat); //4; Check.
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &image[0];
    textureData.RowPitch = width * texturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * height;
    ResourceCommandList rscCommandList = RESOURCE_CMD_LIST;
    UpdateSubresources(rscCommandList.m_resCmdList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rscCommandList.m_resCmdList->ResourceBarrier(1, &resourceBarrier);

    GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue(rscCommandList);
    // 해상도를 낮춰가며 밉맵 생성 필요하다면 추가 필요
   // context->GenerateMips(srv.Get());
}


void D3D12Utils::LoadMetallicRoughnessTexture(
    ComPtr<ID3D12Device> device, const std::string metallicFilename,
    const std::string roughnessFilename, ComPtr<ID3D12Resource>& texture)
{
    if (!metallicFilename.empty() && (metallicFilename == roughnessFilename)) {
        return;
    }
    const string filename = metallicFilename + '_' + roughnessFilename;
    {
        std::unique_lock<std::mutex> lock(s_imageMapMutex);
        if (imageMap.find(filename) == imageMap.end())
        {
            imageMap[filename] = make_unique<ImageInfo>();
        }
    }

    {
        std::unique_lock<std::mutex> lock(imageMap[filename]->imageMutex);
        if (imageMap[filename]->image.size() == 0)
        {
            // 별도 파일일 경우 따로 읽어서 합쳐줍니다.
            // ReadImage()를 활용하기 위해서 두 이미지들을 각각 4채널로 변환 후 다시
            // 3채널로 합치는 방식으로 구현
            int mWidth = 0, mHeight = 0;
            int rWidth = 0, rHeight = 0;
            std::vector<uint8_t> mImage;
            std::vector<uint8_t> rImage;

            // (거의 없겠지만) 둘 중 하나만 있을 경우도 고려하기 위해 각각 파일명
            // 확인
            if (!metallicFilename.empty()) {
                ReadImage(metallicFilename, mImage, mWidth, mHeight);
            }

            if (!roughnessFilename.empty()) {
                ReadImage(roughnessFilename, rImage, rWidth, rHeight);
            }

            // 두 이미지의 해상도가 같다고 가정
            if (!metallicFilename.empty() && !roughnessFilename.empty()) {
                assert(mWidth == rWidth);
                assert(mHeight == rHeight);
            }
            imageMap[filename]->width = mWidth;
            imageMap[filename]->width = mHeight;

            imageMap[filename]->image.resize(size_t(mWidth * mHeight) * 4);
            fill(imageMap[filename]->image.begin(), imageMap[filename]->image.end(), 0);

            for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
                if (rImage.size())
                    imageMap[filename]->image[4 * i + 1] = rImage[4 * i]; // Green = Roughness
                if (mImage.size())
                    imageMap[filename]->image[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
            }
        }
    }
    CreateTextureHelper(device, imageMap[filename]->width, imageMap[filename]->height, imageMap[filename]->image,
        DXGI_FORMAT_R8G8B8A8_UNORM, texture);
}

void D3D12Utils::LoadTexture(const std::wstring path, const bool usSRGB, bool bAsync,
    shared_ptr<Resource> outResource)
{
    std::shared_ptr<Texture> outTexture = outResource->GetTexture();
    if (outTexture == nullptr)
    {
        return;
    }
    if (outTexture->NeedLoading() == false)
    {
        return;
    }

    if (s_resourceMap.find(path) == s_resourceMap.end()
        || s_resourceMap[path].loadType == ELoadType::NotLoaded)
    {
        {
            // 로드하기
            std::unique_lock<std::shared_mutex> writeLock(s_resourceMap[path].resMutex);
            s_resourceMap[path].loadType = ELoadType::Loading;
            s_resourceMap[path].pendingResources.push_back(outResource);
        }
        if (bAsync)
        {
            hlab::ThreadPool& tPool = hlab::ThreadPool::getInstance();
            auto func = [path, usSRGB]() {
                return LoadTextureImpl(path, usSRGB); };
            tPool.EnqueueRenderJob(func);
        }
        else
        {
            LoadTextureImpl(path, usSRGB);
        }

    }
    else if (s_resourceMap[path].loadType == ELoadType::Loading)
    {
        std::unique_lock<std::shared_mutex> writeLock(s_resourceMap[path].resMutex);
        if (s_resourceMap[path].loadType == ELoadType::Loading)
        {
            s_resourceMap[path].pendingResources.push_back(outResource);
        }
        else
        {
            outTexture->CreateFromResource(s_resourceMap[path].resource);
        }
    }
    else if (s_resourceMap[path].loadType == ELoadType::Loaded)
    {
        std::shared_lock<std::shared_mutex> readLock(s_resourceMap[path].resMutex);
        outTexture->CreateFromResource(s_resourceMap[path].resource);
    }
}

void D3D12Utils::CreateTexture(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty,
    D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor, shared_ptr<Resource> outResource)
{
    std::shared_ptr<Texture> outTexture = outResource->GetTexture();
    if (outTexture == nullptr)
    {
        return;
    }
    D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

    
    float arrFloat[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
    D3D12_CLEAR_VALUE optimizedClearValue = CD3DX12_CLEAR_VALUE(resourceDesc.Format, arrFloat);
    D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
    if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    {
        resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
        optimizedClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
        pOptimizedClearValue = &optimizedClearValue;
    }
    else if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
    {
        resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        float arrFloat[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
        optimizedClearValue = CD3DX12_CLEAR_VALUE(resourceDesc.Format, arrFloat);
        pOptimizedClearValue = &optimizedClearValue;
    }
    else
    {
        pOptimizedClearValue = nullptr;
    }
    ComPtr<ID3D12Resource> resource;
    HRESULT hr = DEVICE->CreateCommittedResource(
        &heapProperty,
        heapFlags,
        &resourceDesc,
        resourceStates,
        pOptimizedClearValue,
        IID_PPV_ARGS(&resource));

    assert(SUCCEEDED(hr));

    outTexture->CreateFromResource(resource);
}

void D3D12Utils::LoadTextureImpl(const std::wstring path, const bool usSRGB)
{
    // 동일한 path에 대해서 두번 이상 호출이 안되도록 보장돼야한다.
    ScratchImage image;

    wstring ext = fs::path(path).extension();
    if (ext == L".dds" || ext == L".DDS")
        DirectX::LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, nullptr, image);
    else if (ext == L".tga" || ext == L".TGA")
        DirectX::LoadFromTGAFile(path.c_str(), nullptr, image);
    else if (ext == L"exr" || ext == L".EXR")
        DirectX::LoadFromEXRFile(path.c_str(), nullptr, image);
    else // png, jpg, jpeg, bmp
    {
        LoadTextureNotUsingScratchImage(path, usSRGB);
        return;
    }
        
    ComPtr<ID3D12Resource> texture;
    TexMetadata texMetaData = image.GetMetadata();
    texMetaData.format = usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : texMetaData.format;
    HRESULT hr = DirectX::CreateTexture(DEVICE.Get(), texMetaData, &texture);
    if (FAILED(hr))
        assert(nullptr);

    vector<D3D12_SUBRESOURCE_DATA> subResources;

    hr = DirectX::PrepareUpload(DEVICE.Get(),
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        subResources);

    if (FAILED(hr))
        assert(nullptr);

    const uint64 bufferSize = ::GetRequiredIntermediateSize(texture.Get(), 0, static_cast<uint32>(subResources.size()));

    D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ComPtr<ID3D12Resource> textureUploadHeap;
    hr = DEVICE->CreateCommittedResource(
        &heapProperty,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(textureUploadHeap.GetAddressOf()));

    if (FAILED(hr))
        assert(nullptr);
    ResourceCommandList rscCommandList = RESOURCE_CMD_LIST;
    ::UpdateSubresources(rscCommandList.m_resCmdList.Get(),
        texture.Get(),
        textureUploadHeap.Get(),
        0, 0,
        static_cast<unsigned int>(subResources.size()),
        subResources.data());

    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rscCommandList.m_resCmdList->ResourceBarrier(1, &resourceBarrier);

    //  cpu 리소스 스레드가 gpu를 기다려야되는 문제..
    // -> WaitForSingleObject함수를 통해 Cpu Sleep으로 대기하기때문에 스레드 수를 좀더 늘려서 해결하는게 나을듯 보인다
    GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue(rscCommandList);
    {
        std::unique_lock<std::shared_mutex> writeLock(s_resourceMap[path].resMutex);
        s_resourceMap[path].resource = texture;
        s_resourceMap[path].loadType = ELoadType::Loaded;

        for (shared_ptr<Resource>& resource : s_resourceMap[path].pendingResources)
        {
            std::shared_ptr<Texture> resTex = std::dynamic_pointer_cast<Texture>(resource);
            if (resTex == nullptr)
            {
                continue;
            }
            resTex->CreateFromResource(texture);
        }
        s_resourceMap[path].pendingResources.clear();
    }
}

void D3D12Utils::LoadTextureNotUsingScratchImage(const std::wstring path, const bool usSRGB)
{
    ImageInfo info;
    info.pixelFormat =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    string ext(path.end() - 3, path.end());
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "exr") {
        ReadEXRImage(wstring_to_string(path), info.image, info.width, info.height, info.pixelFormat);
    }
    else {
        ReadImage(wstring_to_string(path), info.image, info.width, info.height);
    }

    // Describe and create a Texture2D.
    ComPtr<ID3D12Resource> texture;
    D3D12_RESOURCE_DESC textureDesc = {};
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = info.width;
    textureDesc.Height = info.height;
    textureDesc.MipLevels = 0;// Check. 1?
    textureDesc.DepthOrArraySize = 1;
    textureDesc.Format = info.pixelFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.SampleDesc.Quality = 1;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)));

    // 스테이징 텍스춰 만들고 CPU에서 이미지를 복사
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    CD3DX12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ComPtr<ID3D12Resource> textureUploadHeap;
    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)));
    const int texturePixelSize = GetPixelSize(info.pixelFormat); //4; Check.
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &info.image[0];
    textureData.RowPitch = info.width * texturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * info.height;
    ResourceCommandList rscCommandList = RESOURCE_CMD_LIST;
    UpdateSubresources(rscCommandList.m_resCmdList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    rscCommandList.m_resCmdList->ResourceBarrier(1, &resourceBarrier);

    GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue(rscCommandList);

    // 해상도를 낮춰가며 밉맵 생성
   // context->GenerateMips(srv.Get());
    // TODO. MipMap 생성 필요

    {
        std::unique_lock<std::shared_mutex> writeLock(s_resourceMap[path].resMutex);
        s_resourceMap[path].resource = texture;
        s_resourceMap[path].loadType = ELoadType::Loaded;

        for (shared_ptr<Resource>& resource : s_resourceMap[path].pendingResources)
        {
            std::shared_ptr<Texture> resTex = std::dynamic_pointer_cast<Texture>(resource);
            if (resTex == nullptr)
            {
                continue;
            }
            resTex->CreateFromResource(texture);
        }
        s_resourceMap[path].pendingResources.clear();
    }

}

void D3D12Utils::LoadAlbedoOpacityTexture(ComPtr<ID3D12Device> device, const std::string albedoFilename,
    const std::string opacityFilename, const bool usSRGB, ComPtr<ID3D12Resource>& texture)
{
    const string filename = albedoFilename + '_' + opacityFilename;
    {
        std::unique_lock<std::mutex> lock(s_imageMapMutex);
        if (imageMap.find(filename) == imageMap.end())
        {
            imageMap[filename] = make_unique<ImageInfo>();
        }
    }

    {
        // 같은 텍스처에 대해선 모든 스레드가 로딩중일때 대기하도록 한다.
        std::unique_lock<std::mutex> lock(imageMap[filename]->imageMutex);
        if (imageMap[filename]->image.size() == 0)
        {
            int width = 0, height = 0;
            std::vector<uint8_t> image;
            imageMap[filename]->pixelFormat =
                usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

            ReadImage(albedoFilename, opacityFilename, imageMap[filename]->image, imageMap[filename]->width, imageMap[filename]->height);
        }
    }
    CreateTextureHelper(device, imageMap[filename]->width, imageMap[filename]->height, imageMap[filename]->image, imageMap[filename]->pixelFormat,
        texture);
}

void D3D12Utils::CreateDDSTexture(
    ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue> commandQueue, const wstring&& filename, bool isCubeMap)
{

    // Create a ResourceUploadBatch for handling resource uploads
    ResourceUploadBatch uploadBatch(device.Get());
    uploadBatch.Begin();

    // Resource for the texture
    ComPtr<ID3D12Resource> texture;

    // Load the texture using DirectXTK's CreateDDSTextureFromFile
    ThrowIfFailed(CreateDDSTextureFromFile(
        device.Get(), uploadBatch, filename.c_str(), texture.ReleaseAndGetAddressOf()));

    // End the upload batch and wait for completion
    auto uploadFinished = uploadBatch.End(commandQueue.Get());
    uploadFinished.wait();

    // Describe and create a SRV (Shader Resource View)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = texture->GetDesc().Format;

    if (texture->GetDesc().DepthOrArraySize == 6 && isCubeMap) {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = texture->GetDesc().MipLevels;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    }
    else {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    }
}

size_t D3D12Utils::GetPixelSize(DXGI_FORMAT pixelFormat) {

    switch (pixelFormat) {
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return sizeof(uint16_t) * 4;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return sizeof(uint32_t) * 4;
    case DXGI_FORMAT_R32_FLOAT:
        return sizeof(uint32_t) * 1;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return sizeof(uint8_t) * 4;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return sizeof(uint8_t) * 4;
    case DXGI_FORMAT_R32_SINT:
        return sizeof(int32_t) * 1;
    case DXGI_FORMAT_R16_FLOAT:
        return sizeof(uint16_t) * 1;
    }

    cout << "PixelFormat not implemented " << pixelFormat << endl;

    return sizeof(uint8_t) * 4;
}


//ComPtr<ID3D11Texture2D> D3D12Utils::CreateStagingTexture(
//    ComPtr<ID3D12Device>& device, ComPtr<ID3D11DeviceContext>& context,
//    const int width, const int height, const std::vector<uint8_t>& image,
//    const DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
//    const int mipLevels = 1, const int arraySize = 1) {

//    // 스테이징 텍스춰 만들기
//    D3D11_TEXTURE2D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.MipLevels = mipLevels;
//    txtDesc.ArraySize = arraySize;
//    txtDesc.Format = pixelFormat;
//    txtDesc.SampleDesc.Count = 1;
//    txtDesc.Usage = D3D11_USAGE_STAGING;
//    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

//    ComPtr<ID3D11Texture2D> stagingTexture;
//    HRESULT stageTextHr = device->CreateTexture2D(&txtDesc, NULL,
//        stagingTexture.GetAddressOf());
//    if (FAILED(stageTextHr)) {
//        cout << "Failed()" << endl;
//        return nullptr;
//    }

//    size_t pixelSize = GetPixelSize(pixelFormat);
//    D3D11_MAPPED_SUBRESOURCE ms;
//    HRESULT  hr = context->Map(stagingTexture.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);

//    uint8_t* pData = (uint8_t*)ms.pData;
//    for (UINT h = 0; h < UINT(height); h++) { // 가로줄 한 줄씩 복사
//        memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
//            width * pixelSize);
//    }
//    context->Unmap(stagingTexture.Get(), 0);

//    return stagingTexture;
//}


} // namespace dengine