#pragma once

#include "D3D11Utils.h"
#include <fp16.h>
#include <random>
#include <vector>

namespace hlab {

using std::vector;

// Âü°í
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Texture.h

class Texture3D {
  public:
    void Initialize(ComPtr<ID3D11Device> &device, UINT width, UINT height,
                    UINT depth, DXGI_FORMAT pixelFormat) {
        Initialize(device, width, height, depth, pixelFormat, {});
    }

    void Initialize(ComPtr<ID3D11Device> &device, UINT width, UINT height,
                    UINT depth, DXGI_FORMAT pixelFormat,
                    const vector<float> &initData) {

        m_width = width;
        m_height = height;
        m_depth = depth;

        D3D11Utils::CreateTexture3D(device, width, height, depth, pixelFormat,
                                    initData, m_texture, m_rtv, m_srv, m_uav);
    }

    void InitNoiseF16(ComPtr<ID3D11Device> &device) {

        using namespace std;

        DXGI_FORMAT_R16G16B16A16_FLOAT;

        const UINT width = 64;
        const UINT height = 1024;
        const UINT depth = 64;

        vector<float> f32(width * height * depth * 4);

        mt19937 gen(0);
        uniform_real_distribution<float> dp(0.0f, 1.0f);
        for (auto &f : f32) {
            f = dp(gen);
        }

        vector<float> f16(f32.size() / 2);

        uint16_t *f16Ptr = (uint16_t *)f16.data();
        for (int i = 0; i < f32.size(); i++) {
            f16Ptr[i] = fp16_ieee_from_fp32_value(f32[i]);
        }

        Initialize(device, width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT,
                   f16);
    }

    void Upload(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const vector<float> &data) {

        D3D11_TEXTURE3D_DESC desc;
        m_texture->GetDesc(&desc);

        if (!m_staging) {
            m_staging = D3D11Utils::CreateStagingTexture3D(
                device, desc.Width, desc.Height, desc.Depth, desc.Format);
        }

        size_t pixelSize = D3D11Utils::GetPixelSize(desc.Format);

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(m_staging.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
        const uint8_t *src = (uint8_t *)data.data();
        uint8_t *dst = (uint8_t *)ms.pData;
        for (UINT k = 0; k < desc.Depth; k++) {
            for (UINT j = 0; j < desc.Height; j++) {
                memcpy(&dst[j * ms.RowPitch + k * ms.DepthPitch],
                       &src[(j * desc.Width + k * desc.Width * desc.Height) *
                            pixelSize],
                       desc.Width * pixelSize);
            }
        }
        context->Unmap(m_staging.Get(), NULL);

        context->CopyResource(m_texture.Get(), m_staging.Get());
    }

    const auto GetTexture() { return m_texture.Get(); }
    const auto GetRTV() { return m_rtv.Get(); }
    const auto GetSRV() { return m_srv.Get(); }
    const auto GetUAV() { return m_uav.Get(); }
    const auto GetAddressOfRTV() { return m_rtv.GetAddressOf(); }
    const auto GetAddressOfSRV() { return m_srv.GetAddressOf(); }
    const auto GetAddressOfUAV() { return m_uav.GetAddressOf(); }

  private:
    UINT m_width = 1;
    UINT m_height = 1;
    UINT m_depth = 1;

    ComPtr<ID3D11Texture3D> m_texture;
    ComPtr<ID3D11Texture3D> m_staging;
    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11ShaderResourceView> m_srv;
    ComPtr<ID3D11UnorderedAccessView> m_uav;
};

} // namespace hlab
