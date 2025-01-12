// #define _CRT_SECURE_NO_WARNINGS // stb_image_write compile error fix

#include "D3D11Utils.h"

#include <DirectXTexEXR.h> // EXR 형식 HDRI 읽기
#include <algorithm>
#include <cctype>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <execution>
#include <fp16.h>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace hlab {

using namespace std;
using namespace DirectX; 
    unordered_map<string, ImageInfo> D3D11Utils::imageMap;
void CheckResult(HRESULT hr, ID3DBlob *errorBlob) {
    if (FAILED(hr)) {
        // 파일이 없을 경우
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            cout << "File not found." << endl;
        }

        // 에러 메시지가 있으면 출력
        if (errorBlob) {
            cout << "Shader compile error\n"
                 << (char *)errorBlob->GetBufferPointer() << endl;
        }
    }
}

void D3D11Utils::CreateVertexShaderAndInputLayout(
    ComPtr<ID3D11Device> &device, wstring filename,
    const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
    ComPtr<ID3D11VertexShader> &m_vertexShader,
    ComPtr<ID3D11InputLayout> &m_inputLayout,
    const vector<D3D_SHADER_MACRO> shaderMacros) {

    ComPtr<ID3DBlob> shaderBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &shaderBlob, &errorBlob);
    CheckResult(hr, errorBlob.Get());

    /*
        // .cso 파일을 읽어들이는 방식, D3DReadFileToBlob() 사용
        wstring path = L"x64/Release/";
    #if defined(DEBUG) || defined(_DEBUG)
        path = L"x64/Debug/";
    #endif
        filename.erase(filename.end() - 4, filename.end()); // 확장자 hlsl 삭제
        filename = path + filename + L"cso";
        HRESULT hr = D3DReadFileToBlob(filename.c_str(),
    shaderBlob.GetAddressOf());
    */
     
    device->CreateVertexShader(shaderBlob->GetBufferPointer(),
                               shaderBlob->GetBufferSize(), NULL,
                               &m_vertexShader);

    device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
                              shaderBlob->GetBufferPointer(),
                              shaderBlob->GetBufferSize(), &m_inputLayout);
}

void D3D11Utils::CreateHullShader(ComPtr<ID3D11Device> &device,
                                  const wstring &filename,
                                  ComPtr<ID3D11HullShader> &m_hullShader) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "hs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateHullShader(shaderBlob->GetBufferPointer(),
                             shaderBlob->GetBufferSize(), NULL, &m_hullShader);
}

void D3D11Utils::CreateDomainShader(
    ComPtr<ID3D11Device> &device, const wstring &filename,
    ComPtr<ID3D11DomainShader> &m_domainShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ds_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateDomainShader(shaderBlob->GetBufferPointer(),
                               shaderBlob->GetBufferSize(), NULL,
                               &m_domainShader);
}

void D3D11Utils::CreatePixelShader(ComPtr<ID3D11Device> &device,
                                   const wstring &filename,
                                   ComPtr<ID3D11PixelShader> &m_pixelShader) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreatePixelShader(shaderBlob->GetBufferPointer(),
                              shaderBlob->GetBufferSize(), NULL,
                              &m_pixelShader);
}

void D3D11Utils::CreateIndexBuffer(ComPtr<ID3D11Device> &device,
                                   const std::vector<uint32_t> &indices,
                                   ComPtr<ID3D11Buffer> &indexBuffer) {
    ThreadPool& tPool = ThreadPool::getInstance();
    auto func = [&device, &indices, &indexBuffer]() {
        return CreateIndexBufferImpl(device, indices, indexBuffer); };
    tPool.EnqueueRenderJob(func);
}

void D3D11Utils::CreateIndexBuffer(ComPtr<ID3D11Device>& device,
    const std::vector<uint32_t>&& indices,
    ComPtr<ID3D11Buffer>& indexBuffer) {
    ThreadPool& tPool = ThreadPool::getInstance();
    auto func = [&device, indices = std::move(indices), &indexBuffer]() {
        return CreateIndexBufferImpl(device, indices, indexBuffer); };
    tPool.EnqueueRenderJob(func);
}

void D3D11Utils::CreateIndexBufferImpl(ComPtr<ID3D11Device>& device,
    const std::vector<uint32_t>& indices,
    ComPtr<ID3D11Buffer>& indexBuffer) {
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

    device->CreateBuffer(&bufferDesc, &indexBufferData,
        indexBuffer.GetAddressOf());
}

void D3D11Utils::CreateGeometryShader(
    ComPtr<ID3D11Device> &device, const wstring &filename,
    ComPtr<ID3D11GeometryShader> &geometryShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "gs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    // CheckResult(hr, errorBlob.Get());

    device->CreateGeometryShader(shaderBlob->GetBufferPointer(),
                                 shaderBlob->GetBufferSize(), NULL,
                                 &geometryShader);
}

void D3D11Utils::CreateComputeShader(
    ComPtr<ID3D11Device> &device, const wstring &filename,
    ComPtr<ID3D11ComputeShader> &computeShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "cs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    // CheckResult(hr, errorBlob.Get());

    ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(),
                                              shaderBlob->GetBufferSize(), NULL,
                                              &computeShader));
}

void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
                  int &width, int &height, DXGI_FORMAT &pixelFormat) {

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
    uint16_t *f16 = (uint16_t *)image.data();
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

void ReadImage(const std::string filename, std::vector<uint8_t> &image,
               int &width, int &height) {
    int channels;

    unsigned char *img =
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
    } else if (channels == 2) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 2; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    } else if (channels == 3) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 3; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    } else if (channels == 4) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    } else {
        std::cout << "Cannot read " << channels << " channels" << endl;
    }

    delete[] img;
}

void ReadImage(const std::string albedoFilename,
               const std::string opacityFilename, std::vector<uint8_t> &image,
               int &width, int &height) {

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

ComPtr<ID3D11Texture2D> D3D11Utils::CreateStagingTexture(
    ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
    const int width, const int height, const std::vector<uint8_t> &image,
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

    uint8_t *pData = (uint8_t *)ms.pData;
    for (UINT h = 0; h < UINT(height); h++) { // 가로줄 한 줄씩 복사
        memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
               width * pixelSize);
    }
    context->Unmap(stagingTexture.Get(), 0);

    return stagingTexture;
}

ComPtr<ID3D11Texture3D> D3D11Utils::CreateStagingTexture3D(
    ComPtr<ID3D11Device> &device, const int width, const int height,
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

void D3D11Utils::CreateTextureHelper(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context, const int width,
                         const int height, const vector<uint8_t> &image,
                         const DXGI_FORMAT pixelFormat,
                         ComPtr<ID3D11Texture2D> &texture,
                         ComPtr<ID3D11ShaderResourceView> &srv) {

    ThreadPool& tPool = ThreadPool::getInstance();
    auto func = [&device, &context, width, height, &image, pixelFormat, &texture, &srv]() {
        return CreateTextureHelperImpl(device, context, width, height, image, pixelFormat
            , texture, srv); };
    tPool.EnqueueRenderJob(func);
}

void D3D11Utils::CreateTextureHelper(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context, const int width,
    const int height, const vector<uint8_t>&& image,
    const DXGI_FORMAT pixelFormat,
    ComPtr<ID3D11Texture2D>& texture,
    ComPtr<ID3D11ShaderResourceView>& srv) {

    ThreadPool& tPool = ThreadPool::getInstance();
    auto func = [&device, &context, width, height, pixelFormat, &texture, &srv,image = std::move(image)]() {
        return CreateTextureHelperImpl(device, context, width, height, image, pixelFormat
            , texture, srv); };
    tPool.EnqueueRenderJob(func);
}

void D3D11Utils::CreateTextureHelperImpl(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context, const int width,
    const int height, const vector<uint8_t>& image,
    const DXGI_FORMAT pixelFormat,
    ComPtr<ID3D11Texture2D>& texture,
    ComPtr<ID3D11ShaderResourceView>& srv) {
    // 스테이징 텍스춰 만들고 CPU에서 이미지를 복사합니다.
    ComPtr<ID3D11Texture2D> stagingTexture = D3D11Utils::CreateStagingTexture(
        device, context, width, height, image, pixelFormat);

    // 실제로 사용할 텍스춰 설정
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = 0; // 밉맵 레벨 최대
    txtDesc.ArraySize = 1;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용
    txtDesc.CPUAccessFlags = 0;

    // 초기 데이터 없이 텍스춰 생성 (전부 검은색)
    device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf());

    // 실제로 생성된 MipLevels를 확인해보고 싶을 경우
    // texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // 스테이징 텍스춰로부터 가장 해상도가 높은 이미지 복사
    context->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0,
        stagingTexture.Get(), 0, NULL);

    // ResourceView 만들기
    device->CreateShaderResourceView(texture.Get(), 0, srv.GetAddressOf());

    // 해상도를 낮춰가며 밉맵 생성
    context->GenerateMips(srv.Get());

    // HLSL 쉐이더 안에서는 SampleLevel() 사용
}


void D3D11Utils::LoadMetallicRoughnessTexture(
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
void D3D11Utils::CreateTexture(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context,
                               const std::string filename, const bool usSRGB,
                               ComPtr<ID3D11Texture2D> &tex,
                               ComPtr<ID3D11ShaderResourceView> &srv) {
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


void D3D11Utils::CreateTexture(ComPtr<ID3D11Device>& device,
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

void D3D11Utils::CreateUATexture(ComPtr<ID3D11Device> &device, const int width,
                                 const int height,
                                 const DXGI_FORMAT pixelFormat,
                                 ComPtr<ID3D11Texture2D> &texture,
                                 ComPtr<ID3D11RenderTargetView> &rtv,
                                 ComPtr<ID3D11ShaderResourceView> &srv,
                                 ComPtr<ID3D11UnorderedAccessView> &uav) {

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

void D3D11Utils::CreateTexture3D(ComPtr<ID3D11Device> &device, const int width,
                                 const int height, const int depth,
                                 const DXGI_FORMAT pixelFormat,
                                 const vector<float> &initData,
                                 ComPtr<ID3D11Texture3D> &texture,
                                 ComPtr<ID3D11RenderTargetView> &rtv,
                                 ComPtr<ID3D11ShaderResourceView> &srv,
                                 ComPtr<ID3D11UnorderedAccessView> &uav) {

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
    } else {
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

void D3D11Utils::CreateStagingBuffer(ComPtr<ID3D11Device> &device,
                                     const UINT numElements,
                                     const UINT sizeElement,
                                     const void *initData,
                                     ComPtr<ID3D11Buffer> &buffer) {

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
    } else {
        ThrowIfFailed(device->CreateBuffer(&desc, NULL, buffer.GetAddressOf()));
    }
}

void D3D11Utils::CopyFromStagingBuffer(ComPtr<ID3D11DeviceContext> &context,
                                       ComPtr<ID3D11Buffer> &buffer, UINT size,
                                       void *dest) {
    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(buffer.Get(), NULL, D3D11_MAP_READ, NULL, &ms);
    memcpy(dest, ms.pData, size);
    context->Unmap(buffer.Get(), NULL);
}

void D3D11Utils::CreateStructuredBuffer(
    ComPtr<ID3D11Device> &device, const UINT numElements,
    const UINT sizeElement, const void *initData, ComPtr<ID3D11Buffer> &buffer,
    ComPtr<ID3D11ShaderResourceView> &srv,
    ComPtr<ID3D11UnorderedAccessView> &uav) {

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
    } else {
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

void D3D11Utils::CreateIndirectArgsBuffer(ComPtr<ID3D11Device> &device,
                                          const UINT numElements,
                                          const UINT sizeElement,
                                          const void *initData,
                                          ComPtr<ID3D11Buffer> &buffer) {

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
    } else {
        ThrowIfFailed(device->CreateBuffer(&desc, NULL, buffer.GetAddressOf()));
    }
}

void D3D11Utils::CreateAppendBuffer(ComPtr<ID3D11Device> &device,
                                    const UINT numElements,
                                    const UINT sizeElement,
                                    const void *initData,
                                    ComPtr<ID3D11Buffer> &buffer,
                                    ComPtr<ID3D11ShaderResourceView> &srv,
                                    ComPtr<ID3D11UnorderedAccessView> &uav) {

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
    } else {
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

void D3D11Utils::CreateTextureArray(
    ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
    const std::vector<std::string> filenames, ComPtr<ID3D11Texture2D> &texture,
    ComPtr<ID3D11ShaderResourceView> &textureSRV) {

    using namespace std;

    if (filenames.empty())
        return;

    // 모든 이미지의 width와 height가 같다고 가정합니다.

    // 파일로부터 이미지 여러 개를 읽어들입니다.
    int width = 0, height = 0;
    vector<vector<uint8_t>> imageArray;
    for (const auto &f : filenames) {

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

        auto &image = imageArray[i];

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

void D3D11Utils::CreateDDSTexture(
    ComPtr<ID3D11Device>& device, const wstring&& filename, bool isCubeMap,
    ComPtr<ID3D11ShaderResourceView>& textureResourceView) {

    ThreadPool& tPool = ThreadPool::getInstance();
    auto func = [&device, filename = std::move(filename), &isCubeMap, &textureResourceView]() {
        return CreateDDSTextureImpl(device, std::move(filename), isCubeMap, textureResourceView); };
    std::cout << "CreateDDSTexture" << std::endl;
    tPool.EnqueueRenderJob(func);
}

void D3D11Utils::CreateDDSTextureImpl(
    ComPtr<ID3D11Device> &device, const wstring&& filename, bool isCubeMap,
    ComPtr<ID3D11ShaderResourceView> &textureResourceView) {
    std::cout << "CreateDDSTextureImpl" << std::endl;
    ComPtr<ID3D11Texture2D> texture;

    UINT miscFlags = 0;
    if (isCubeMap) {
        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    // https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
    ThrowIfFailed(CreateDDSTextureFromFileEx(
        device.Get(), filename.c_str(), 0, D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, DDS_LOADER_FLAGS(false),
        (ID3D11Resource **)texture.GetAddressOf(),
        textureResourceView.GetAddressOf(), NULL));
}

void D3D11Utils::WriteToPngFile(ComPtr<ID3D11Device> &device,
                                ComPtr<ID3D11DeviceContext> &context,
                                ComPtr<ID3D11Texture2D> &textureToWrite,
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
    uint8_t *pData = (uint8_t *)ms.pData;
    for (unsigned int h = 0; h < desc.Height; h++) {
        memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch],
               desc.Width * sizeof(uint8_t) * 4);
    }

    context->Unmap(stagingTexture.Get(), NULL);

    stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4, pixels.data(),
                   desc.Width * 4);

    cout << filename << endl;
}

size_t D3D11Utils::GetPixelSize(DXGI_FORMAT pixelFormat) {

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