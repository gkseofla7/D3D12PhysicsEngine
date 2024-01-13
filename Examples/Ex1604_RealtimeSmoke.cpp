#include "Ex1604_RealtimeSmoke.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <limits>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1604_RealtimeSmoke::Ex1604_RealtimeSmoke() : AppBase() {}

bool Ex1604_RealtimeSmoke::InitScene() {

    cout << "Ex1604_RealtimeSmoke::InitScene()" << endl;

    AppBase::m_camera.Reset(Vector3(0.0f, 0.0f, -4.22895f), 0.0f, 0.0f);
    AppBase::InitCubemaps(
        L"../Assets/Textures/Cubemaps/HDRI/", L"clear_pureskyEnvHDR.dds",
        L"clear_pureskySpecularHDR.dds", L"clear_pureskyDiffuseHDR.dds",
        L"clear_pureskyBrdf.dds");
    AppBase::m_globalConstsCPU.strengthIBL = 0.1f;
    AppBase::InitScene();

    m_upScale = 2;

    // 컴퓨터가 느리면 해상도를 낮추세요 (예: 128/2, 64/2, 64/2)
    m_width = 128;
    m_height = 64;
    m_depth = 64;

    m_widthUp = m_width * m_upScale;
    m_heightUp = m_height * m_upScale;
    m_depthUp = m_depth * m_upScale;

    // Initialize fluid const buffer
    m_fluidConsts.m_cpu.time = 0.0f;
    m_fluidConsts.m_cpu.dt = 1 / 60.0f;
    m_fluidConsts.m_cpu.dxBase =
        Vector3(1.0f / m_width, 1.0f / m_height, 1.0f / m_depth);
    m_fluidConsts.m_cpu.dxUp =
        Vector3(1.0f / m_widthUp, 1.0f / m_heightUp, 1.0f / m_depthUp);
    m_fluidConsts.m_cpu.upScale = m_upScale;
    m_fluidConsts.Initialize(m_device);

    // Initialize shaders
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_AdvectionCS.hlsl",
                                    m_advectionCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_ApplyPressureCS.hlsl",
                                    m_applyPressureCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_DivergenceCS.hlsl",
                                    m_divergenceCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_JacobiCS.hlsl",
                                    m_jacobiCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_SourcingCS.hlsl",
                                    m_sourcingCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_DownSampleCS.hlsl",
                                    m_downSampleCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1604_DiffUpSampleCS.hlsl",
                                    m_diffUpSampleCS);
    D3D11Utils::CreateComputeShader(m_device,
                                    L"Ex1604_VorticityConfinementCS.hlsl",
                                    m_vorticityConfinementCS);

    m_velocity.Initialize(m_device, m_width, m_height, m_depth,
                          DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_velocityTemp.Initialize(m_device, m_width, m_height, m_depth,
                              DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_pressure.Initialize(m_device, m_width, m_height, m_depth,
                          DXGI_FORMAT_R16_FLOAT);
    m_pressureTemp.Initialize(m_device, m_width, m_height, m_depth,
                              DXGI_FORMAT_R16_FLOAT);
    m_divergence.Initialize(m_device, m_width, m_height, m_depth,
                            DXGI_FORMAT_R16_FLOAT);
    m_density.Initialize(m_device, m_width, m_height, m_depth,
                         DXGI_FORMAT_R16_FLOAT);
    m_densityTemp.Initialize(m_device, m_width, m_height, m_depth,
                             DXGI_FORMAT_R16_FLOAT);

    m_bc.Initialize(m_device, m_width, m_height, m_depth, DXGI_FORMAT_R32_SINT);

    // Textures for upsampling
    m_velocityUp.Initialize(m_device, m_widthUp, m_heightUp, m_depthUp,
                            DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_velocityUpTemp.Initialize(m_device, m_widthUp, m_heightUp, m_depthUp,
                                DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_densityUp.Initialize(m_device, m_widthUp, m_heightUp, m_depthUp,
                           DXGI_FORMAT_R16_FLOAT);
    m_densityUpTemp.Initialize(m_device, m_widthUp, m_heightUp, m_depthUp,
                               DXGI_FORMAT_R16_FLOAT);

    // Volume 쉐이더는 별도로 초기화
    Graphics::InitVolumeShaders(m_device);
    m_volumeConsts.m_cpu.densityAbsorption = 10.0;
    m_volumeConsts.Initialize(m_device);

    Vector3 center(0.0f);
    m_volumeModel = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeBox(1.0f)});
    m_volumeModel->UpdateWorldRow(
        Matrix::CreateScale(2.0f * Vector3(1.0f, float(m_height) / m_width,
                                           float(m_depth) / m_width)) *
        Matrix::CreateTranslation(center));
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);
    m_volumeModel->m_meshes.front()->densityTex = m_densityUp; // 고해상도
    m_volumeModel->m_meshes.front()->lightingTex.Initialize(
        m_device, m_width, m_height, m_depth, DXGI_FORMAT_R16_FLOAT, {});

    m_volumeModel->m_isPickable = true;

    AppBase::m_basicList.push_back(m_volumeModel); // 마우스 선택

    // Rendering
    m_context->CSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
    ID3D11SamplerState *samplerStates[5] = {
        Graphics::pointClampSS.Get(), Graphics::linearClampSS.Get(),
        Graphics::linearMirrorSS.Get(), Graphics::pointWrapSS.Get(),
        Graphics::linearWrapSS.Get()};
    m_context->CSSetSamplers(0, 5, samplerStates);

    return true;
}

void Ex1604_RealtimeSmoke::Update(float dt) {

    AppBase::Update(dt);
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    if (!AppBase::m_pauseAnimation) {

        const int numSubsteps = 2;

        for (int i = 0; i < numSubsteps; i++) {
            m_fluidConsts.m_cpu.dt = 1.0f / 60.0f / numSubsteps;
            m_fluidConsts.m_cpu.time += m_fluidConsts.m_cpu.dt;
            m_fluidConsts.m_cpu.numNewParticles = 0; // 꺼놓음
            m_fluidConsts.Upload(m_context);

            DownSample();

            Sourcing();

            Projection();

            DiffUpSample();

            Advection();
        }
    }
}

void Ex1604_RealtimeSmoke::DownSample() {

    // Apply vorticity confiment to up
    {
        m_context->CopyResource(m_velocityUpTemp.GetTexture(),
                                m_velocityUp.GetTexture());

        ID3D11ShaderResourceView *srvs[2] = {m_velocityUpTemp.GetSRV(),
                                             m_densityUp.GetSRV()};
        ID3D11UnorderedAccessView *uavs[1] = {m_velocityUp.GetUAV()};
        m_context->CSSetShaderResources(0, 2, srvs);
        m_context->CSSetUnorderedAccessViews(0, 1, uavs, NULL);
        m_context->CSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
        m_context->CSSetShader(m_vorticityConfinementCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_widthUp / 16.0f)),
                            UINT(ceil(m_heightUp / 16.0f)),
                            UINT(ceil(m_depthUp / 4.0f)));
        AppBase::ComputeShaderBarrier();
    }

    // Downsampling
    {
        ID3D11ShaderResourceView *srvs[2] = {m_velocityUp.GetSRV(),
                                             m_densityUp.GetSRV()};
        ID3D11UnorderedAccessView *uavs[2] = {m_velocity.GetUAV(),
                                              m_density.GetUAV()};
        m_context->CSSetShaderResources(0, 2, srvs);
        m_context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
        m_context->CSSetShader(m_downSampleCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                            UINT(ceil(m_height / 16.0f)),
                            UINT(ceil(m_depth / 4.0f)));
        AppBase::ComputeShaderBarrier();
    }

    // Backup old
    m_context->CopyResource(m_velocityTemp.GetTexture(),
                            m_velocity.GetTexture());
    m_context->CopyResource(m_densityTemp.GetTexture(), m_density.GetTexture());
}

void Ex1604_RealtimeSmoke::Sourcing() {

    ID3D11UnorderedAccessView *uavs[3] = {m_velocity.GetUAV(),
                                          m_density.GetUAV(), m_bc.GetUAV()};
    m_context->CSSetUnorderedAccessViews(0, 3, uavs, NULL);
    m_context->CSSetShader(m_sourcingCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                        UINT(ceil(m_height / 16.0f)),
                        UINT(ceil(m_depth / 4.0f)));
    AppBase::ComputeShaderBarrier();
}

void Ex1604_RealtimeSmoke::Projection() {

    // Compute divergence
    m_context->CSSetShaderResources(0, 1, m_velocity.GetAddressOfSRV());
    m_context->CSSetShaderResources(2, 1, m_bc.GetAddressOfSRV());
    ID3D11UnorderedAccessView *uavs[3] = {
        m_divergence.GetUAV(), m_pressure.GetUAV(), m_pressureTemp.GetUAV()};
    m_context->CSSetUnorderedAccessViews(0, 3, uavs, NULL);
    m_context->CSSetShader(m_divergenceCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                        UINT(ceil(m_height / 16.0f)),
                        UINT(ceil(m_depth / 4.0f)));
    AppBase::ComputeShaderBarrier();

    // Jacobi iteration
    m_context->CSSetShader(m_jacobiCS.Get(), 0, 0);
    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            m_context->CSSetShaderResources(0, 1, m_pressure.GetAddressOfSRV());
            m_context->CSSetUnorderedAccessViews(
                0, 1, m_pressureTemp.GetAddressOfUAV(), NULL);
        } else {
            m_context->CSSetShaderResources(0, 1,
                                            m_pressureTemp.GetAddressOfSRV());
            m_context->CSSetUnorderedAccessViews(
                0, 1, m_pressure.GetAddressOfUAV(), NULL);
        }
        m_context->CSSetShaderResources(1, 1, m_divergence.GetAddressOfSRV());
        m_context->CSSetShaderResources(2, 1, m_bc.GetAddressOfSRV());
        m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                            UINT(ceil(m_height / 16.0f)),
                            UINT(ceil(m_depth / 4.0f)));
        AppBase::ComputeShaderBarrier();
    }

    // Apply pressure
    m_context->CSSetShaderResources(0, 1, m_pressure.GetAddressOfSRV());
    m_context->CSSetShaderResources(2, 1, m_bc.GetAddressOfSRV());
    m_context->CSSetUnorderedAccessViews(0, 1, m_velocity.GetAddressOfUAV(),
                                         NULL);
    m_context->CSSetShader(m_applyPressureCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                        UINT(ceil(m_height / 16.0f)),
                        UINT(ceil(m_depth / 4.0f)));
    AppBase::ComputeShaderBarrier();
}

void Ex1604_RealtimeSmoke::DiffUpSample() {

    // Run with Up-res
    ID3D11ShaderResourceView *srvs[4] = {
        m_velocityTemp.GetSRV(), m_velocity.GetSRV(), m_densityTemp.GetSRV(),
        m_density.GetSRV()};
    ID3D11UnorderedAccessView *uavs[2] = {m_velocityUp.GetUAV(),
                                          m_densityUp.GetUAV()};
    m_context->CSSetShaderResources(0, 4, srvs);
    m_context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
    m_context->CSSetShader(m_diffUpSampleCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_widthUp / 16.0f)),
                        UINT(ceil(m_heightUp / 16.0f)),
                        UINT(ceil(m_depthUp / 4.0f)));
    AppBase::ComputeShaderBarrier();
}

void Ex1604_RealtimeSmoke::Advection() {

    // Advect Upsampled velocity/density

    m_context->CopyResource(m_velocityUpTemp.GetTexture(),
                            m_velocityUp.GetTexture());
    m_context->CopyResource(m_densityUpTemp.GetTexture(),
                            m_densityUp.GetTexture());

    ID3D11ShaderResourceView *srvs[2] = {m_velocityUpTemp.GetSRV(),
                                         m_densityUpTemp.GetSRV()};
    ID3D11UnorderedAccessView *uavs[2] = {m_velocityUp.GetUAV(),
                                          m_densityUp.GetUAV()};
    m_context->CSSetShaderResources(0, 2, srvs);
    m_context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
    m_context->CSSetShader(m_advectionCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_widthUp / 16.0f)),
                        UINT(ceil(m_heightUp / 16.0f)),
                        UINT(ceil(m_depthUp / 4.0f)));
    AppBase::ComputeShaderBarrier();
}

void Ex1604_RealtimeSmoke::Render() {

    // AppBase::Render()에서 그려지지 않도록 설정
    m_volumeModel->m_isVisible = false;

    AppBase::Render();

    m_volumeModel->m_isVisible = true;

    // Draw volume

    AppBase::SetPipelineState(Graphics::volumeSmokePSO);
    m_context->PSSetConstantBuffers(3, 1, m_volumeConsts.GetAddressOf());
    m_volumeModel->Render(m_context);

    // Draw bounding box

    AppBase::SetPipelineState(Graphics::boundingBoxPSO);
    m_context->OMSetRenderTargets(1, AppBase::m_floatRTV.GetAddressOf(),
                                  NULL); // Depth 끄기
    m_volumeModel->RenderWireBoundingBox(m_context);

    AppBase::PostRender();
}

void Ex1604_RealtimeSmoke::UpdateGUI() {
    AppBase::UpdateGUI();

    ImGui::SliderFloat("Turbulence", &m_fluidConsts.m_cpu.turbulence, 0.0f,
                       1.5f);
    ImGui::SliderFloat("Source", &m_fluidConsts.m_cpu.sourceStrength, 0.1f,
                       3.0f);
    ImGui::SliderFloat("Buoyancy", &m_fluidConsts.m_cpu.buoyancy, 0.0f, 10.0f);
}

} // namespace hlab