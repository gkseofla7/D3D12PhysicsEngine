#include "StableFluids.h"

namespace hlab {

void StableFluids::Initialize(ComPtr<ID3D11Device> &device, const UINT width,
                              const UINT height) {

    m_width = width;
    m_height = height;

    // Update const buffer
    m_constsCPU.dt = 0.0f;
    m_constsCPU.viscosity = 1000.0f;
    m_constsCPU.sourcingVelocity = Vector2(-0.1f, 0.0f);
    m_constsCPU.sourcingDensity = Vector4(1.0f);
    m_constsCPU.i = -1;
    m_constsCPU.j = -1;

    // Initialize const buffer
    D3D11Utils::CreateConstBuffer(device, m_constsCPU, m_constsGPU);

    // Initialize shaders
    D3D11Utils::CreateComputeShader(device, L"Ex1601_AdvectionCS.hlsl",
                                    m_advectionCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_ApplyPressureCS.hlsl",
                                    m_applyPressureCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_DiffuseCS.hlsl",
                                    m_diffuseCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_DivergenceCS.hlsl",
                                    m_divergenceCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_JacobiCS.hlsl",
                                    m_jacobiCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_SourcingCS.hlsl",
                                    m_sourcingCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_ComputeVorticityCS.hlsl",
                                    m_computeVorticityCS);
    D3D11Utils::CreateComputeShader(device, L"Ex1601_ConfineVorticityCS.hlsl",
                                    m_confineVorticityCS);

    // Initialize Textures

    m_velocity.Initialize(device, width, height, DXGI_FORMAT_R16G16_FLOAT);
    m_velocityTemp.Initialize(device, width, height, DXGI_FORMAT_R16G16_FLOAT);

    m_pressure.Initialize(device, width, height, DXGI_FORMAT_R16_FLOAT);
    m_pressureTemp.Initialize(device, width, height, DXGI_FORMAT_R16_FLOAT);
    m_divergence.Initialize(device, width, height, DXGI_FORMAT_R16_FLOAT);
    m_vorticity.Initialize(device, width, height, DXGI_FORMAT_R16_FLOAT);

    m_density.Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_densityTemp.Initialize(device, width, height,
                             DXGI_FORMAT_R16G16B16A16_FLOAT);
}

void StableFluids::Update(ComPtr<ID3D11Device> &device,
                          ComPtr<ID3D11DeviceContext> &context, float dt) {

    m_constsCPU.dt = dt;

    D3D11Utils::UpdateBuffer(context, m_constsCPU, m_constsGPU);
    context->CSSetConstantBuffers(0, 1, m_constsGPU.GetAddressOf());

    ID3D11SamplerState *samplerStates[2] = {Graphics::pointWrapSS.Get(),
                                            Graphics::linearWrapSS.Get()};
    context->CSSetSamplers(0, 2, samplerStates);

    Sourcing(context);
    Diffuse(context);
    Projection(context);
    Advection(context);
}

void StableFluids::ComputeShaderBarrier(ComPtr<ID3D11DeviceContext> &context) {
    ID3D11ShaderResourceView *nullSRV[2] = {0, 0};
    context->CSSetShaderResources(0, 2, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[2] = {0, 0};
    context->CSSetUnorderedAccessViews(0, 2, nullUAV, NULL);
}

void StableFluids::Sourcing(ComPtr<ID3D11DeviceContext> &context) {

    ID3D11UnorderedAccessView *uavs[2] = {m_velocity.GetUAV(),
                                          m_density.GetUAV()};
    context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
    context->CSSetShader(m_sourcingCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);

    // Vorticity confinemenet
    context->CSSetShaderResources(0, 1, m_velocity.GetAddressOfSRV());
    context->CSSetUnorderedAccessViews(0, 1, m_vorticity.GetAddressOfUAV(),
                                       NULL);
    context->CSSetShader(m_computeVorticityCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);

    context->CSSetShaderResources(0, 1, m_vorticity.GetAddressOfSRV());
    context->CSSetUnorderedAccessViews(0, 1, m_velocity.GetAddressOfUAV(),
                                       NULL);
    context->CSSetShader(m_confineVorticityCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);
}

void StableFluids::Diffuse(ComPtr<ID3D11DeviceContext> &context) {

    ID3D11ShaderResourceView *evenSRVs[2] = {m_velocity.GetSRV(),
                                             m_density.GetSRV()};
    ID3D11ShaderResourceView *oddSRVs[2] = {m_velocityTemp.GetSRV(),
                                            m_densityTemp.GetSRV()};
    ID3D11UnorderedAccessView *evenUAVs[2] = {m_velocityTemp.GetUAV(),
                                              m_densityTemp.GetUAV()};
    ID3D11UnorderedAccessView *oddUAVs[2] = {m_velocity.GetUAV(),
                                             m_density.GetUAV()};

    context->CSSetShader(m_diffuseCS.Get(), 0, 0);

    for (int i = 0; i < 10; i++) {

        if (i % 2 == 0) {
            context->CSSetShaderResources(0, 2, evenSRVs);
            context->CSSetUnorderedAccessViews(0, 2, evenUAVs, NULL);
        } else {
            context->CSSetShaderResources(0, 2, oddSRVs);
            context->CSSetUnorderedAccessViews(0, 2, oddUAVs, NULL);
        }

        context->Dispatch(UINT(ceil(m_width / 32.0f)),
                          UINT(ceil(m_height / 32.0f)), 1);

        ComputeShaderBarrier(context);
    }
}

void StableFluids::Projection(ComPtr<ID3D11DeviceContext> &context) {

    // Compute divergence

    context->CSSetShaderResources(0, 1, m_velocity.GetAddressOfSRV());

    ID3D11UnorderedAccessView *uavs[3] = {
        m_divergence.GetUAV(), m_pressure.GetUAV(), m_pressureTemp.GetUAV()};

    context->CSSetUnorderedAccessViews(0, 3, uavs, NULL);
    context->CSSetShader(m_divergenceCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);

    // Jacobi iteration

    context->CSSetShader(m_jacobiCS.Get(), 0, 0);

    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            context->CSSetShaderResources(0, 1, m_pressure.GetAddressOfSRV());
            context->CSSetUnorderedAccessViews(
                0, 1, m_pressureTemp.GetAddressOfUAV(), NULL);
        } else {
            context->CSSetShaderResources(0, 1,
                                          m_pressureTemp.GetAddressOfSRV());
            context->CSSetUnorderedAccessViews(
                0, 1, m_pressure.GetAddressOfUAV(), NULL);
        }
        context->CSSetShaderResources(1, 1, m_divergence.GetAddressOfSRV());
        context->Dispatch(UINT(ceil(m_width / 32.0f)),
                          UINT(ceil(m_height / 32.0f)), 1);
        ComputeShaderBarrier(context);
    }

    // Apply pressure
    context->CSSetShaderResources(0, 1, m_pressure.GetAddressOfSRV());
    context->CSSetUnorderedAccessViews(0, 1, m_velocity.GetAddressOfUAV(),
                                       NULL);
    context->CSSetShader(m_applyPressureCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);
}

void StableFluids::Advection(ComPtr<ID3D11DeviceContext> &context) {

    context->CopyResource(m_velocityTemp.GetTexture(), m_velocity.GetTexture());
    context->CopyResource(m_densityTemp.GetTexture(), m_density.GetTexture());

    ID3D11ShaderResourceView *srvs[2] = {m_velocityTemp.GetSRV(),
                                         m_densityTemp.GetSRV()};
    ID3D11UnorderedAccessView *uavs[2] = {m_velocity.GetUAV(),
                                          m_density.GetUAV()};

    context->CSSetShaderResources(0, 2, srvs);
    context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);

    context->CSSetShader(m_advectionCS.Get(), 0, 0);
    context->Dispatch(UINT(ceil(m_width / 32.0f)), UINT(ceil(m_height / 32.0f)),
                      1);
    ComputeShaderBarrier(context);
}

} // namespace hlab