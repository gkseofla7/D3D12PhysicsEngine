#pragma once

#include "D3D11Utils.h"

namespace hlab {

// 참고
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Texture.h
class Texture2D {
  public:
    void Initialize(ComPtr<ID3D11Device> &device, UINT width, UINT height,
                    DXGI_FORMAT pixelFormat) {
        m_width = width;
        m_height = height;

        D3D11Utils::CreateUATexture(device, width, height, pixelFormat,
                                    m_texture, m_rtv, m_srv, m_uav);
    }

    void Upload(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                const vector<uint8_t> &data) {

        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);

        if (!m_staging) {
            m_staging = D3D11Utils::CreateStagingTexture(
                device, context, desc.Width, desc.Height, data, desc.Format,
                desc.MipLevels, desc.ArraySize);
        } else {

            size_t pixelSize = D3D11Utils::GetPixelSize(desc.Format);

            D3D11_MAPPED_SUBRESOURCE ms;
            context->Map(m_staging.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);

            const uint8_t *src = (uint8_t *)data.data();
            uint8_t *dst = (uint8_t *)ms.pData;
            for (UINT j = 0; j < desc.Height; j++) {
                memcpy(&dst[j * ms.RowPitch],
                       &src[(j * desc.Width) * pixelSize],
                       desc.Width * pixelSize);
            }

            context->Unmap(m_staging.Get(), NULL);
        }

        context->CopyResource(m_texture.Get(), m_staging.Get());
    }

    void Download(ComPtr<ID3D11DeviceContext> &context,
                  vector<uint8_t> &buffer) {

        context->CopyResource(m_staging.Get(), m_texture.Get());

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(m_staging.Get(), NULL, D3D11_MAP_READ, NULL,
                     &ms); // D3D11_MAP_READ 주의
        memcpy(buffer.data(), (uint8_t *)ms.pData, buffer.size());
        context->Unmap(m_staging.Get(), NULL);
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

    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11Texture2D> m_staging;
    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11ShaderResourceView> m_srv;
    ComPtr<ID3D11UnorderedAccessView> m_uav;
};

} // namespace hlab
