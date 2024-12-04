// #define _CRT_SECURE_NO_WARNINGS // stb_image_write compile error fix
#include "D3D12Utils.h"
#include "GraphicsCommonD3D12.h"

#include <DirectXTexEXR.h> // EXR 형식 HDRI 읽기
#include <algorithm>
#include <cctype>
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <execution>
#include <fp16.h>
#include <iostream>
#include <string>

#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace hlab {

    using namespace std;
    using namespace DirectX;
    unordered_map<string, ImageInfo> D3D12Utils::imageMap;
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

    void D3D12Utils::CreateVertexShader(
        ComPtr<ID3D12Device>& device, wstring filename,
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

    void D3D12Utils::CreatePixelShader(ComPtr<ID3D12Device>& device,
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

    void D3D12Utils::CreateRootSignature(ComPtr<ID3D12Device>& device,
        ComPtr<ID3D12RootSignature>& rootSignature, vector<CD3DX12_ROOT_PARAMETER1>& rootParams)
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParams.size()), rootParams.data(), 0, nullptr, rootSignatureFlags);


        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    }

    void D3D12Utils::CreatePipelineState(ComPtr<ID3D12Device>& device, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc, ComPtr<ID3D12PipelineState>& OutPipelineState)
    {
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&OutPipelineState)));
    }


    void D3D12Utils::CreateIndexBuffer(ComPtr<ID3D12Device>& device,
        const std::vector<uint32_t>& indices,
        D3D12_INDEX_BUFFER_VIEW& OutIndexBufferView) {
        ThreadPool& tPool = ThreadPool::getInstance();
        auto func = [&device, &indices, &OutIndexBufferView]() {
            return CreateIndexBufferImpl(device, indices, OutIndexBufferView); };
        tPool.EnqueueRenderJob(func);
    }

    void D3D12Utils::CreateIndexBuffer(ComPtr<ID3D12Device>& device,
        const std::vector<uint32_t>&& indices,
        D3D12_INDEX_BUFFER_VIEW& OutIndexBufferView) {
        ThreadPool& tPool = ThreadPool::getInstance();
        auto func = [&device, indices = std::move(indices), &OutIndexBufferView]() {
            return CreateIndexBufferImpl(device, indices, OutIndexBufferView); };
        tPool.EnqueueRenderJob(func);
    }

    void D3D12Utils::CreateIndexBufferImpl(ComPtr<ID3D12Device>& device,
        const std::vector<uint32_t>& indices,
        D3D12_INDEX_BUFFER_VIEW& OutIndexBufferView) {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
        bufferDesc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(uint32_t);

        D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
        indexBufferData.pSysMem = indices.data();
        indexBufferData.SysMemPitch = 0;
        indexBufferData.SysMemSlicePitch = 0;

        const UINT indexBufferSize = sizeof(indices);// TODO. 여기도 확인필요
        // TODO. D3D12_HEAP_TYPE_UPLOAD -> D3D12_HEAP_TYPE_DEFAULT 변경하는게
        // m_IndexBuffer를 따로 받는쪽에서 관리하는게 필요할까?
        ComPtr<ID3D12Resource> m_IndexBuffer;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_IndexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, indices.data(), sizeof(indices));
        m_IndexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        OutIndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
        OutIndexBufferView.SizeInBytes = indexBufferSize;
        OutIndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
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

    ComPtr<ID3D11Texture2D> D3D12Utils::CreateStagingTexture(
        ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const int width, const int height, const std::vector<uint8_t>& image,
        const DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        const int mipLevels = 1, const int arraySize = 1) {

        // 스테이징 텍스춰 만들기
        D3D11_TEXTURE2D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.MipLevels = mipLevels;
        txtDesc.ArraySize = arraySize;
        txtDesc.Format = pixelFormat;
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Usage = D3D11_USAGE_STAGING;
        txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

        ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT stageTextHr = device->CreateTexture2D(&txtDesc, NULL,
            stagingTexture.GetAddressOf());
        if (FAILED(stageTextHr)) {
            cout << "Failed()" << endl;
            return nullptr;
        }

        size_t pixelSize = GetPixelSize(pixelFormat);
        D3D11_MAPPED_SUBRESOURCE ms;
        HRESULT  hr = context->Map(stagingTexture.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);

        uint8_t* pData = (uint8_t*)ms.pData;
        for (UINT h = 0; h < UINT(height); h++) { // 가로줄 한 줄씩 복사
            memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
                width * pixelSize);
        }
        context->Unmap(stagingTexture.Get(), 0);

        return stagingTexture;
    }

    ComPtr<ID3D11Texture3D> D3D12Utils::CreateStagingTexture3D(
        ComPtr<ID3D11Device>& device, const int width, const int height,
        const int depth, const DXGI_FORMAT pixelFormat) {

        // 스테이징 텍스춰 만들기
        D3D11_TEXTURE3D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.Depth = depth;
        txtDesc.MipLevels = 1;
        txtDesc.Format = pixelFormat;
        txtDesc.Usage = D3D11_USAGE_STAGING;
        txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

        ComPtr<ID3D11Texture3D> stagingTexture;
        if (FAILED(device->CreateTexture3D(&txtDesc, NULL,
            stagingTexture.GetAddressOf()))) {
            cout << "CreateStagingTexture3D() failed." << endl;
        }

        return stagingTexture;
    }

    void D3D12Utils::CreateTextureHelper(ComPtr<ID3D12Device>& device,
        ComPtr<ID3D12GraphicsCommandList>& commandList,
        ComPtr<ID3D12CommandQueue>& commandQueue, 
        const int width, const int height, const vector<uint8_t>& image,
        const DXGI_FORMAT pixelFormat,
        ComPtr<ID3D12Resource>& texture,
        CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) {

        ThreadPool& tPool = ThreadPool::getInstance();
        auto func = [&device, &commandList, &commandQueue , &srvHandle, width, height, &image, pixelFormat, &texture]() {
            return CreateTextureHelperImpl(device, commandList, commandQueue, width, height, image, pixelFormat
                , texture, srvHandle); };
        tPool.EnqueueRenderJob(func);
    }

    void D3D12Utils::CreateTextureHelper(ComPtr<ID3D12Device>& device,
        ComPtr<ID3D12GraphicsCommandList>& commandList, 
        ComPtr<ID3D12CommandQueue>& commandQueue,
        const int width, const int height, const vector<uint8_t>&& image,
        const DXGI_FORMAT pixelFormat,
        ComPtr<ID3D12Resource>& texture,
        CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) {

        ThreadPool& tPool = ThreadPool::getInstance();
        auto func = [&device, &commandList, &commandQueue, &srvHandle,width, height, pixelFormat, &texture, image = std::move(image)]() {
            return CreateTextureHelperImpl(device, commandList, commandQueue, width, height, image, pixelFormat
                , texture, srvHandle); };
        tPool.EnqueueRenderJob(func);
    }

    void D3D12Utils::CreateTextureHelperImpl(ComPtr<ID3D12Device>& device,
        ComPtr<ID3D12GraphicsCommandList>& commandList,
        ComPtr<ID3D12CommandQueue>& commandQueue,
        const int width, const int height, const vector<uint8_t>& image,
        const DXGI_FORMAT pixelFormat,
        ComPtr<ID3D12Resource>& texture,
        CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle) {
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
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
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
        // Check. 이렇게 스택 메모리에 쌓이는데 비동기 실행하면 이거 문제 되지 않나?
        // -> 아 바로 명령어 실행하고 대기..ㅎ
        ComPtr<ID3D12Resource> textureUploadHeap;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));
        const int texturePixelSize = GetPixelSize(pixelFormat); //4; Check.
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &image[0];
        textureData.RowPitch = width * texturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * height;

        UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
        DGraphics::RegisterSrvHeap(texture, &srvDesc, srvHandle);

        ThrowIfFailed(commandList->Close());
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        // 해상도를 낮춰가며 밉맵 생성
       // context->GenerateMips(srv.Get());
        // TODO. MipMap 생성 필요

    }


    void D3D12Utils::CreateMetallicRoughnessTexture(
        ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const std::string metallicFilename, const std::string roughnessFilename,
        ComPtr<ID3D11Texture2D>& texture, ComPtr<ID3D11ShaderResourceView>& srv)
    {
        std::cout << "CreateMetallicRoughnessTextureImpl" << std::endl;
        string filename = metallicFilename + '_' + roughnessFilename;
        if (imageMap.find(filename) == imageMap.end())
        {
            imageMap[filename] = ImageInfo();
            // GLTF 방식은 이미 합쳐져 있음
            if (!metallicFilename.empty() && (metallicFilename == roughnessFilename)) {
                CreateTexture(device, context, metallicFilename, false, texture, srv);
            }
            else {
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
                imageMap[filename].width = mWidth;
                imageMap[filename].width = mHeight;

                imageMap[filename].image.resize(size_t(mWidth * mHeight) * 4);
                fill(imageMap[filename].image.begin(), imageMap[filename].image.end(), 0);

                for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
                    if (rImage.size())
                        imageMap[filename].image[4 * i + 1] = rImage[4 * i]; // Green = Roughness
                    if (mImage.size())
                        imageMap[filename].image[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
                }
            }
            CreateTextureHelper(device, context, imageMap[filename].width, imageMap[filename].height, imageMap[filename].image,
                DXGI_FORMAT_R8G8B8A8_UNORM, texture, srv);
        }
    }
    void D3D12Utils::CreateTexture(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const std::string filename, const bool usSRGB,
        ComPtr<ID3D11Texture2D>& tex,
        ComPtr<ID3D11ShaderResourceView>& srv) {
        if (imageMap.find(filename) == imageMap.end())
        {
            imageMap[filename] = ImageInfo();
            imageMap[filename];
            imageMap[filename].pixelFormat =
                usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

            string ext(filename.end() - 3, filename.end());
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == "exr") {
                ReadEXRImage(filename, imageMap[filename].image, imageMap[filename].width, imageMap[filename].height, imageMap[filename].pixelFormat);
            }
            else {
                ReadImage(filename, imageMap[filename].image, imageMap[filename].width, imageMap[filename].height);
            }
        }
        CreateTextureHelper(device, context, imageMap[filename].width, imageMap[filename].height, imageMap[filename].image, imageMap[filename].pixelFormat, tex,
            srv);
    }


    void D3D12Utils::CreateTexture(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const std::string albedoFilename,
        const std::string opacityFilename,
        const bool usSRGB,
        ComPtr<ID3D11Texture2D>& texture,
        ComPtr<ID3D11ShaderResourceView>& srv) {
        string filename = albedoFilename + '_' + opacityFilename;
        if (imageMap.find(filename) == imageMap.end())
        {
            imageMap[filename] = ImageInfo();
            imageMap[filename];
            int width = 0, height = 0;
            std::vector<uint8_t> image;
            imageMap[filename].pixelFormat =
                usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

            ReadImage(albedoFilename, opacityFilename, imageMap[filename].image, imageMap[filename].width, imageMap[filename].height);
        }
        CreateTextureHelper(device, context, imageMap[filename].width, imageMap[filename].height, imageMap[filename].image, imageMap[filename].pixelFormat,
            texture, srv);
    }

    void D3D12Utils::CreateUATexture(ComPtr<ID3D11Device>& device, const int width,
        const int height,
        const DXGI_FORMAT pixelFormat,
        ComPtr<ID3D11Texture2D>& texture,
        ComPtr<ID3D11RenderTargetView>& rtv,
        ComPtr<ID3D11ShaderResourceView>& srv,
        ComPtr<ID3D11UnorderedAccessView>& uav) {

        D3D11_TEXTURE2D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.MipLevels = 1;
        txtDesc.ArraySize = 1;
        txtDesc.Format = pixelFormat; // 주로 FLOAT 사용
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Usage = D3D11_USAGE_DEFAULT;
        txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET |
            D3D11_BIND_UNORDERED_ACCESS;
        txtDesc.MiscFlags = 0;
        txtDesc.CPUAccessFlags = 0;

        ThrowIfFailed(
            device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf()));
        ThrowIfFailed(device->CreateRenderTargetView(texture.Get(), NULL,
            rtv.GetAddressOf()));
        ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), NULL,
            srv.GetAddressOf()));
        ThrowIfFailed(device->CreateUnorderedAccessView(texture.Get(), NULL,
            uav.GetAddressOf()));
    }

    void D3D12Utils::CreateTexture3D(ComPtr<ID3D11Device>& device, const int width,
        const int height, const int depth,
        const DXGI_FORMAT pixelFormat,
        const vector<float>& initData,
        ComPtr<ID3D11Texture3D>& texture,
        ComPtr<ID3D11RenderTargetView>& rtv,
        ComPtr<ID3D11ShaderResourceView>& srv,
        ComPtr<ID3D11UnorderedAccessView>& uav) {

        D3D11_TEXTURE3D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.Depth = depth;
        txtDesc.MipLevels = 1;
        txtDesc.Format = pixelFormat;
        txtDesc.Usage = D3D11_USAGE_DEFAULT;
        txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET |
            D3D11_BIND_UNORDERED_ACCESS;
        txtDesc.MiscFlags = 0;
        txtDesc.CPUAccessFlags = 0;

        if (initData.size() > 0) {
            size_t pixelSize = GetPixelSize(pixelFormat);
            D3D11_SUBRESOURCE_DATA bufferData;
            ZeroMemory(&bufferData, sizeof(bufferData));
            bufferData.pSysMem = initData.data();
            bufferData.SysMemPitch = UINT(width * pixelSize);
            bufferData.SysMemSlicePitch = UINT(width * height * pixelSize);
            ThrowIfFailed(device->CreateTexture3D(&txtDesc, &bufferData,
                texture.GetAddressOf()));
        }
        else {
            ThrowIfFailed(
                device->CreateTexture3D(&txtDesc, NULL, texture.GetAddressOf()));
        }

        ThrowIfFailed(device->CreateRenderTargetView(texture.Get(), NULL,
            rtv.GetAddressOf()));
        ThrowIfFailed(device->CreateShaderResourceView(texture.Get(), NULL,
            srv.GetAddressOf()));
        ThrowIfFailed(device->CreateUnorderedAccessView(texture.Get(), NULL,
            uav.GetAddressOf()));
    }

    void D3D12Utils::CreateStagingBuffer(ComPtr<ID3D11Device>& device,
        const UINT numElements,
        const UINT sizeElement,
        const void* initData,
        ComPtr<ID3D11Buffer>& buffer) {

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = numElements * sizeElement;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        desc.StructureByteStride = sizeElement;

        if (initData) {
            D3D11_SUBRESOURCE_DATA bufferData;
            ZeroMemory(&bufferData, sizeof(bufferData));
            bufferData.pSysMem = initData;
            ThrowIfFailed(
                device->CreateBuffer(&desc, &bufferData, buffer.GetAddressOf()));
        }
        else {
            ThrowIfFailed(device->CreateBuffer(&desc, NULL, buffer.GetAddressOf()));
        }
    }

    void D3D12Utils::CopyFromStagingBuffer(ComPtr<ID3D11DeviceContext>& context,
        ComPtr<ID3D11Buffer>& buffer, UINT size,
        void* dest) {
        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_READ, NULL, &ms);
        memcpy(dest, ms.pData, size);
        context->Unmap(buffer.Get(), NULL);
    }

    void D3D12Utils::CreateStructuredBuffer(
        ComPtr<ID3D11Device>& device, const UINT numElements,
        const UINT sizeElement, const void* initData, ComPtr<ID3D11Buffer>& buffer,
        ComPtr<ID3D11ShaderResourceView>& srv,
        ComPtr<ID3D11UnorderedAccessView>& uav) {

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = numElements * sizeElement;
        bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | // Compute Shader
            D3D11_BIND_SHADER_RESOURCE;   // Vertex Shader
        bufferDesc.StructureByteStride = sizeElement;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

        // 참고: Structured는 D3D11_BIND_VERTEX_BUFFER로 사용 불가

        if (initData) {
            D3D11_SUBRESOURCE_DATA bufferData;
            ZeroMemory(&bufferData, sizeof(bufferData));
            bufferData.pSysMem = initData;
            ThrowIfFailed(device->CreateBuffer(&bufferDesc, &bufferData,
                buffer.GetAddressOf()));
        }
        else {
            ThrowIfFailed(
                device->CreateBuffer(&bufferDesc, NULL, buffer.GetAddressOf()));
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ZeroMemory(&uavDesc, sizeof(uavDesc));
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = numElements;
        device->CreateUnorderedAccessView(buffer.Get(), &uavDesc,
            uav.GetAddressOf());

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.BufferEx.NumElements = numElements;
        device->CreateShaderResourceView(buffer.Get(), &srvDesc,
            srv.GetAddressOf());
    }

    void D3D12Utils::CreateIndirectArgsBuffer(ComPtr<ID3D11Device>& device,
        const UINT numElements,
        const UINT sizeElement,
        const void* initData,
        ComPtr<ID3D11Buffer>& buffer) {

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = numElements * sizeElement;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags =
            D3D11_BIND_UNORDERED_ACCESS; // ComputeShader에서 업데이트 가능
        desc.StructureByteStride = sizeElement;
        desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // <- IndirectArgs

        if (initData) {
            D3D11_SUBRESOURCE_DATA bufferData;
            ZeroMemory(&bufferData, sizeof(bufferData));
            bufferData.pSysMem = initData;
            ThrowIfFailed(
                device->CreateBuffer(&desc, &bufferData, buffer.GetAddressOf()));
        }
        else {
            ThrowIfFailed(device->CreateBuffer(&desc, NULL, buffer.GetAddressOf()));
        }
    }

    void D3D12Utils::CreateAppendBuffer(ComPtr<ID3D11Device>& device,
        const UINT numElements,
        const UINT sizeElement,
        const void* initData,
        ComPtr<ID3D11Buffer>& buffer,
        ComPtr<ID3D11ShaderResourceView>& srv,
        ComPtr<ID3D11UnorderedAccessView>& uav) {

        // CreateStructuredBuffer()와 비교했을 때 UAV 생성 시
        // D3D11_BUFFER_UAV_FLAG_APPEND 사용 한 가지만 다름

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = numElements * sizeElement;
        bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | // Compute Shader
            D3D11_BIND_SHADER_RESOURCE;   // Vertex Shader
        bufferDesc.StructureByteStride = sizeElement;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

        if (initData) {
            D3D11_SUBRESOURCE_DATA bufferData;
            ZeroMemory(&bufferData, sizeof(bufferData));
            bufferData.pSysMem = initData;
            ThrowIfFailed(device->CreateBuffer(&bufferDesc, &bufferData,
                buffer.GetAddressOf()));
        }
        else {
            ThrowIfFailed(
                device->CreateBuffer(&bufferDesc, NULL, buffer.GetAddressOf()));
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ZeroMemory(&uavDesc, sizeof(uavDesc));
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = numElements;
        uavDesc.Buffer.Flags =
            D3D11_BUFFER_UAV_FLAG_APPEND; // <- AppendBuffer로 사용
        device->CreateUnorderedAccessView(buffer.Get(), &uavDesc,
            uav.GetAddressOf());

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.BufferEx.NumElements = numElements;
        device->CreateShaderResourceView(buffer.Get(), &srvDesc,
            srv.GetAddressOf());
    }

    void D3D12Utils::CreateTextureArray(
        ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const std::vector<std::string> filenames, ComPtr<ID3D11Texture2D>& texture,
        ComPtr<ID3D11ShaderResourceView>& textureSRV) {

        using namespace std;

        if (filenames.empty())
            return;

        // 모든 이미지의 width와 height가 같다고 가정합니다.

        // 파일로부터 이미지 여러 개를 읽어들입니다.
        int width = 0, height = 0;
        vector<vector<uint8_t>> imageArray;
        for (const auto& f : filenames) {

            cout << f << endl;

            std::vector<uint8_t> image;

            ReadImage(f, image, width, height);

            imageArray.push_back(image);
        }

        UINT size = UINT(filenames.size());

        // Texture2DArray를 만듭니다. 이때 데이터를 CPU로부터 복사하지 않습니다.
        D3D11_TEXTURE2D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = UINT(width);
        txtDesc.Height = UINT(height);
        txtDesc.MipLevels = 0; // 밉맵 레벨 최대
        txtDesc.ArraySize = size;
        txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        txtDesc.SampleDesc.Count = 1;
        txtDesc.SampleDesc.Quality = 0;
        txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
        txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용

        // 초기 데이터 없이 텍스춰를 만듭니다.
        device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf());

        // 실제로 만들어진 MipLevels를 확인
        texture->GetDesc(&txtDesc);
        // cout << txtDesc.MipLevels << endl;

        // StagingTexture를 만들어서 하나씩 복사합니다.
        for (size_t i = 0; i < imageArray.size(); i++) {

            auto& image = imageArray[i];

            // StagingTexture는 Texture2DArray가 아니라 Texture2D 입니다.
            ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
                device, context, width, height, image, txtDesc.Format, 1, 1);

            // 스테이징 텍스춰를 텍스춰 배열의 해당 위치에 복사합니다.
            UINT subresourceIndex =
                D3D11CalcSubresource(0, UINT(i), txtDesc.MipLevels);

            context->CopySubresourceRegion(texture.Get(), subresourceIndex, 0, 0, 0,
                stagingTexture.Get(), 0, NULL);
        }

        device->CreateShaderResourceView(texture.Get(), NULL,
            textureSRV.GetAddressOf());

        context->GenerateMips(textureSRV.Get());
    }

    void D3D12Utils::CreateDDSTexture(
        ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue> commandQueue, 
        const wstring&& filename, bool isCubeMap,
        CD3DX12_GPU_DESCRIPTOR_HANDLE& textureResourceView) {

        ThreadPool& tPool = ThreadPool::getInstance();
        auto func = [&device, &commandQueue,filename = std::move(filename), &isCubeMap, &textureResourceView]() {
            return CreateDDSTextureImpl(device, commandQueue, std::move(filename), isCubeMap, textureResourceView); };
        std::cout << "CreateDDSTexture" << std::endl;
        tPool.EnqueueRenderJob(func);
    }

    void D3D12Utils::CreateDDSTextureImpl(
        ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandQueue> commandQueue, const wstring&& filename, bool isCubeMap,
        CD3DX12_GPU_DESCRIPTOR_HANDLE& textureResourceView) {
        
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
        DGraphics::RegisterSrvHeap(texture, &srvDesc, textureResourceView);

    }

    void D3D12Utils::WriteToPngFile(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        ComPtr<ID3D11Texture2D>& textureToWrite,
        const std::string filename) {

        D3D11_TEXTURE2D_DESC desc;
        textureToWrite->GetDesc(&desc);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU에서 읽기 가능
        desc.Usage = D3D11_USAGE_STAGING; // GPU에서 CPU로 보낼 데이터를 임시 보관

        ComPtr<ID3D11Texture2D> stagingTexture;
        if (FAILED(device->CreateTexture2D(&desc, NULL,
            stagingTexture.GetAddressOf()))) {
            cout << "Failed()" << endl;
        }

        // 참고: 전체 복사할 때
        // context->CopyResource(stagingTexture.Get(), pTemp.Get());

        // 일부만 복사할 때 사용
        D3D11_BOX box;
        box.left = 0;
        box.right = desc.Width;
        box.top = 0;
        box.bottom = desc.Height;
        box.front = 0;
        box.back = 1;
        context->CopySubresourceRegion(stagingTexture.Get(), 0, 0, 0, 0,
            textureToWrite.Get(), 0, &box);

        // R8G8B8A8 이라고 가정
        std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);
        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(stagingTexture.Get(), NULL, D3D11_MAP_READ, NULL,
            &ms); // D3D11_MAP_READ 주의

        // 텍스춰가 작을 경우에는
        // ms.RowPitch가 width * sizeof(uint8_t) * 4보다 클 수도 있어서
        // for문으로 가로줄 하나씩 복사
        uint8_t* pData = (uint8_t*)ms.pData;
        for (unsigned int h = 0; h < desc.Height; h++) {
            memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch],
                desc.Width * sizeof(uint8_t) * 4);
        }

        context->Unmap(stagingTexture.Get(), NULL);

        stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4, pixels.data(),
            desc.Width * 4);

        cout << filename << endl;
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

} // namespace hlab