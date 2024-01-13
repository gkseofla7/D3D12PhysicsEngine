#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1402_Blur : public AppBase {
  public:
    Ex1402_Blur();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    void PrepareForStagingTexture();
    void ComputeShaderBlur(const bool useGroupCache);
    void PixelShaderBlur();

    ComPtr<ID3D11Texture2D> m_stagingTexture;
    ComPtr<ID3D11Texture2D> m_texA, m_texB;
    ComPtr<ID3D11RenderTargetView> m_rtvA, m_rtvB;
    ComPtr<ID3D11ShaderResourceView> m_srvA, m_srvB;
    ComPtr<ID3D11UnorderedAccessView> m_uavA, m_uavB;

    ComPtr<ID3D11ComputeShader> m_blurXCS;
    ComPtr<ID3D11ComputeShader> m_blurYCS;
    ComPtr<ID3D11ComputeShader> m_blurXGroupCacheCS;
    ComPtr<ID3D11ComputeShader> m_blurYGroupCacheCS;

    ComputePSO m_blurXComputePSO;
    ComputePSO m_blurYComputePSO;
    ComputePSO m_blurXGroupCacheComputePSO;
    ComputePSO m_blurYGroupCacheComputePSO;

    ComPtr<ID3D11PixelShader> m_blurXPS;
    ComPtr<ID3D11PixelShader> m_blurYPS;

    GraphicsPSO m_blurXPixelPSO;   // ComputeShader와 비교용
    GraphicsPSO m_blurYPixelPSO;   // ComputeShader와 비교용
    shared_ptr<Mesh> m_screenMesh; // PS Blur에 사용
};

} // namespace hlab