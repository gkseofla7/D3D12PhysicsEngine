#include "Ex1606_HybridWater.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "MarchingCubes.h"

#include <limits>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1606_HybridWater::Ex1606_HybridWater() : AppBase() {}

bool Ex1606_HybridWater::InitScene() {

    cout << "Ex1606_HybridWater::InitScene()" << endl;

    AppBase::m_camera.Reset(Vector3(0.00207493f, 0.0f, -4.31722f), 0.00981733f,
                            0.00436338f);

    AppBase::InitCubemaps(
        L"../Assets/Textures/Cubemaps/HDRI/", L"clear_pureskyEnvHDR.dds",
        L"clear_pureskySpecularHDR.dds", L"clear_pureskyDiffuseHDR.dds",
        L"clear_pureskyBrdf.dds");
    AppBase::m_globalConstsCPU.strengthIBL = 0.5f;
    AppBase::InitScene();

    m_upScale = 1; // 구현 안됨

    m_width = m_height = m_depth = 128;

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
    m_fluidConsts.m_cpu.sourceStrength = 0.8f;
    m_fluidConsts.m_cpu.numNewParticles = 3096;
    m_fluidConsts.m_cpu.width = m_width;
    m_fluidConsts.m_cpu.height = m_height;
    m_fluidConsts.m_cpu.depth = m_depth;
    m_fluidConsts.Initialize(m_device);

    // Initialize shaders
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_ApplyPressureCS.hlsl",
                                    m_applyPressureCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_DivergenceCS.hlsl",
                                    m_divergenceCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_JacobiCS.hlsl",
                                    m_jacobiCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_DiffUpSampleCS.hlsl",
                                    m_diffUpSampleCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_ParticleStepCS.hlsl",
                                    m_particleStepCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_FirstIndexCS.hlsl",
                                    m_firstIndexCS);
    D3D11Utils::CreateComputeShader(m_device, L"Ex1606_ParticleToGridCS.hlsl",
                                    m_particleToGridCS);

    D3D11Utils::CreatePixelShader(m_device, L"Ex1606_SignedDistancePS.hlsl",
                                  m_signedDistancePS);

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
    m_firstIndex.Initialize(m_device, m_width, m_height, m_depth,
                            DXGI_FORMAT_R32_UINT);
    m_bc.Initialize(m_device, m_width, m_height, m_depth, DXGI_FORMAT_R32_SINT);
    m_signedDistance.Initialize(m_device, m_width, m_height, m_depth,
                                DXGI_FORMAT_R16_FLOAT);

    // Noise texture
    m_noise.InitNoiseF16(m_device);

    // Volume 쉐이더는 별도로 초기화
    Graphics::InitVolumeShaders(m_device);
    m_volumeConsts.m_cpu.densityAbsorption = 10.0;
    m_volumeConsts.Initialize(m_device);

    Vector3 center(0.0f);
    m_volumeModel = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeBox(1.0f)});
    m_volumeModel->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
    m_volumeModel->m_materialConsts.GetCpu().emissionFactor = Vector3(0.8f);
    m_volumeModel->m_materialConsts.GetCpu().metallicFactor = 0.1f;
    m_volumeModel->m_materialConsts.GetCpu().roughnessFactor = 0.9f;
    m_volumeModel->UpdateWorldRow(
        Matrix::CreateScale(1.0f * Vector3(1.0f, float(m_height) / m_width,
                                           float(m_depth) / m_width)) *
        Matrix::CreateTranslation(center));
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);
    m_volumeModel->m_meshes.front()->densityTex = m_density; // 고해상도
    m_volumeModel->m_meshes.front()->lightingTex.Initialize(
        m_device, m_width, m_height, m_depth, DXGI_FORMAT_R16_FLOAT, {});

    m_volumeModel->m_isPickable = true;

    AppBase::m_basicList.push_back(m_volumeModel);

    // Particles
    m_particles.m_cpu.resize(1024 * 1024); // 최대 입자 개수
    std::mt19937 gen(0);
    std::uniform_real_distribution<float> dp(-1.0f, 1.0f);
    for (auto &p : m_particles.m_cpu) {
        p.pos = Vector3(0.0f);
    }
    m_particles.Initialize(m_device);

    m_numActiveParticles.m_cpu.resize(1);
    m_numActiveParticles.Initialize(m_device);
    m_numActiveParticles.m_cpu[0] = 0;
    m_numActiveParticles.Upload(m_context);

    m_sort.Initialize(m_device, UINT(m_particles.m_cpu.size()),
                      L"BitonicSortCS.hlsl");
    vector<BitonicSort::Element> initData(m_particles.m_cpu.size());
    for (uint32_t i = 0; i < initData.size(); i++) {
        initData[i].key = std::numeric_limits<uint32_t>::max(); // INACTIVE
        initData[i].value = i;
    }
    m_sort.m_array.Upload(m_context, initData);

    // Rendering

    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1606_StructuredBufferVS.hlsl", inputElements,
        m_particleVS, m_inputLayout);
    D3D11Utils::CreateVertexBuffer(
        m_device,
        vector<Vector3>(std::min(m_particles.m_cpu.size(), // Particles용 dummy
                                 size_t(m_widthUp * m_heightUp *
                                        m_depthUp))), // Marching Cubes용 dummy
        m_dummyVertexBuffer);
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1606_MarchingCubesVS.hlsl", inputElements, m_mcVS,
        m_inputLayout);
    D3D11Utils::CreateGeometryShader(m_device, L"Ex1606_MarchingCubesGS.hlsl",
                                     m_mcGS);

    // Pixel Shader
    D3D11Utils::CreatePixelShader(m_device, L"Ex1606_StructuredBufferPS.hlsl",
                                  m_particlePS);

    m_context->CSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
    ID3D11SamplerState *samplerStates[5] = {
        Graphics::pointClampSS.Get(), Graphics::linearClampSS.Get(),
        Graphics::linearMirrorSS.Get(), Graphics::pointWrapSS.Get(),
        Graphics::linearWrapSS.Get()};
    m_context->CSSetSamplers(0, 5, samplerStates);

    // 모든 샘플러들을 공통으로 사용
    m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->GSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->CSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());

    // 16, 256 순서 주의 (2차원 배열의 메모리 배치 고려)
    m_triTable.Initialize(m_device, 16, 256, DXGI_FORMAT_R32_SINT);
    vector<uint8_t> table8(16 * 256 * sizeof(int));
    memcpy(table8.data(), triTable, table8.size());
    m_triTable.Upload(m_device, m_context, table8);

    return true;
}

void Ex1606_HybridWater::Update(float dt) {

    AppBase::Update(dt);
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    if (!AppBase::m_pauseAnimation) {

        const int numSubsteps = 2;

        for (int i = 0; i < numSubsteps; i++) {
            m_fluidConsts.m_cpu.dt = 1.0f / 60.0f / numSubsteps;
            m_fluidConsts.m_cpu.time += m_fluidConsts.m_cpu.dt;
            m_fluidConsts.Upload(m_context);

            Projection();
            ParticleStep();
        }
    }
}

void Ex1606_HybridWater::Projection() {

    // Backup old
    m_context->CopyResource(m_velocityTemp.GetTexture(),
                            m_velocity.GetTexture());

    // Compute divergence and determine boundary conditions
    m_context->CSSetShaderResources(0, 1, m_velocityTemp.GetAddressOfSRV());
    ID3D11UnorderedAccessView *uavs[5] = {
        m_divergence.GetUAV(), m_pressure.GetUAV(), m_pressureTemp.GetUAV(),
        m_density.GetUAV(), m_bc.GetUAV()};
    m_context->CSSetUnorderedAccessViews(0, 5, uavs, NULL);
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

void Ex1606_HybridWater::ParticleStep() {

    // Update particle velocity from grid velocity
    {
        // UpdateParticles와 합치면 속도가 많이 느려짐
        ID3D11ShaderResourceView *srvs[4] = {
            m_velocityTemp.GetSRV(), m_velocity.GetSRV(), m_density.GetSRV(),
            m_signedDistance.GetSRV()};
        ID3D11UnorderedAccessView *uavs[2] = {m_particles.GetUAV(),
                                              m_sort.m_array.GetUAV()};
        m_context->CSSetShaderResources(0, 4, srvs);
        m_context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
        m_context->CSSetShader(m_diffUpSampleCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_numActiveParticles.m_cpu[0] / 1024.0f)),
                            1, 1);
        AppBase::ComputeShaderBarrier();
    }

    // Advect particles, add new particles.
    {
        ID3D11ShaderResourceView *srvs[2] = {m_noise.GetSRV(),
                                             m_numActiveParticles.GetSRV()};
        ID3D11UnorderedAccessView *uavs[2] = {m_particles.GetUAV(),
                                              m_sort.m_array.GetUAV()};
        m_context->CSSetShaderResources(0, 2, srvs);
        m_context->CSSetUnorderedAccessViews(0, 2, uavs, NULL);
        m_context->CSSetShader(m_particleStepCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_particles.m_cpu.size() / 1024.0f)), 1,
                            1);
        AppBase::ComputeShaderBarrier();
    }

    // Sort particles by grid index
    m_sort.SortGPU(m_device, m_context);

    // Update first indices
    {
        ID3D11UnorderedAccessView *uavs[4] = {
            m_particles.GetUAV(), m_sort.m_array.GetUAV(),
            m_numActiveParticles.GetUAV(), m_firstIndex.GetUAV()};
        m_context->CSSetUnorderedAccessViews(0, 4, uavs, NULL);
        m_context->CSSetShader(m_firstIndexCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_particles.m_cpu.size() / 1024.0f)), 1,
                            1);
        AppBase::ComputeShaderBarrier();
        m_numActiveParticles.Download(m_context);
    }

    // ParticleToGrid (density, velocity, and signed distances)
    {
        ID3D11ShaderResourceView *srvs[3] = {m_particles.GetSRV(),
                                             m_sort.m_array.GetSRV(),
                                             m_firstIndex.GetSRV()};
        ID3D11UnorderedAccessView *uavs[3] = {
            m_velocity.GetUAV(), m_density.GetUAV(), m_signedDistance.GetUAV()};
        m_context->CSSetShaderResources(0, 3, srvs);
        m_context->CSSetUnorderedAccessViews(0, 3, uavs, NULL);
        m_context->CSSetShader(m_particleToGridCS.Get(), 0, 0);
        m_context->Dispatch(UINT(ceil(m_width / 16.0f)),
                            UINT(ceil(m_height / 16.0f)),
                            UINT(ceil(m_depth / 4.0f)));
        AppBase::ComputeShaderBarrier();
    }
}

void Ex1606_HybridWater::Render() {

    m_volumeModel->m_isVisible = false;

    AppBase::Render();

    m_volumeModel->m_isVisible = true;

    // Draw volume

    if (m_renderDensity) {
        AppBase::SetPipelineState(Graphics::volumeSmokePSO);
        m_volumeModel->m_meshes[0]->densityTex = m_density;
        m_context->PSSetConstantBuffers(3, 1, m_volumeConsts.GetAddressOf());
        m_volumeModel->Render(m_context);
    }

    ID3D11Buffer *constBuffers[2] = {
        m_volumeModel->m_meshes[0]->meshConstsGPU.Get(),
        m_volumeModel->m_meshes[0]->materialConstsGPU.Get()};
    m_context->VSSetConstantBuffers(1, 2, constBuffers);
    m_context->PSSetConstantBuffers(1, 2, constBuffers);
    m_context->GSSetConstantBuffers(1, 2, constBuffers);

    // Draw particles
    if (m_renderParticles) {
        AppBase::SetMainViewport();
        UINT strides = 0, offsets = 0;
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        m_context->IASetVertexBuffers(0, 1, m_dummyVertexBuffer.GetAddressOf(),
                                      &strides, &offsets);
        m_context->VSSetShader(m_particleVS.Get(), 0, 0);
        m_context->VSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
        m_context->PSSetShader(m_particlePS.Get(), 0, 0);
        m_context->CSSetShader(NULL, 0, 0);
        m_context->VSSetConstantBuffers(
            1, 1, m_volumeModel->m_meshes[0]->meshConstsGPU.GetAddressOf());
        m_context->VSSetShaderResources(0, 1, m_particles.GetAddressOfSRV());
        m_context->VSSetShaderResources(1, 1, m_sort.m_array.GetAddressOfSRV());
        m_context->PSSetShaderResources(5, 1,
                                        m_signedDistance.GetAddressOfSRV());
        m_context->OMSetRenderTargets(1, m_floatRTV.GetAddressOf(),
                                      m_defaultDSV.Get());
        m_context->Draw(UINT(m_numActiveParticles.m_cpu[0]), 0);

        ID3D11ShaderResourceView *nulls[2] = {0, 0};
        m_context->VSSetShaderResources(0, 2, nulls);
        m_context->PSSetShaderResources(5, 2, nulls);
    }

    // Marching Cubes
    if (m_renderMarchingCubes) {
        if (m_renderWired)
            AppBase::SetPipelineState(Graphics::defaultWirePSO);
        else
            AppBase::SetPipelineState(Graphics::defaultSolidPSO);
        AppBase::SetMainViewport();
        UINT strides = 0, offsets = 0;
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        m_context->IASetVertexBuffers(0, 1, m_dummyVertexBuffer.GetAddressOf(),
                                      &strides, &offsets);
        m_context->VSSetShader(m_mcVS.Get(), 0, 0);
        m_context->GSSetShader(m_mcGS.Get(), 0, 0);
        ID3D11ShaderResourceView *srvs[3] = {
            m_density.GetSRV(), m_signedDistance.GetSRV(), m_triTable.GetSRV()};
        m_context->GSSetShaderResources(0, 3, srvs);
        m_context->GSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
        m_context->OMSetRenderTargets(1, m_floatRTV.GetAddressOf(),
                                      m_defaultDSV.Get());
        m_context->Draw(UINT(m_widthUp * m_heightUp * m_depthUp), 0);

        ID3D11ShaderResourceView *nulls[2] = {0, 0};
        m_context->GSSetShaderResources(0, 2, nulls);
        m_context->GSSetShader(NULL, 0, 0);
    }

    // Raycasting signed distance directly
    if (m_renderRayCasting) {
        m_volumeModel->m_meshes[0]->densityTex = m_signedDistance;
        AppBase::SetPipelineState(Graphics::volumeSmokePSO);
        m_context->PSSetConstantBuffers(4, 1, m_fluidConsts.GetAddressOf());
        m_context->PSSetShader(m_signedDistancePS.Get(), 0, 0);
        m_context->PSSetConstantBuffers(3, 1, m_volumeConsts.GetAddressOf());
        m_volumeModel->Render(m_context);
    }

    // Draw bounding box
    {
        AppBase::SetPipelineState(Graphics::boundingBoxPSO);
        m_context->OMSetRenderTargets(1, AppBase::m_floatRTV.GetAddressOf(),
                                      m_defaultDSV.Get());
        m_volumeModel->RenderWireBoundingBox(m_context);
    }

    AppBase::PostRender();
}

void Ex1606_HybridWater::UpdateGUI() {
    AppBase::UpdateGUI();

    ImGui::SliderFloat("Source", &m_fluidConsts.m_cpu.sourceStrength, 0.2f,
                       1.5f);
    ImGui::Checkbox("RenderParticles", &m_renderParticles);
    ImGui::Checkbox("RenderDensity", &m_renderDensity);
    ImGui::Checkbox("RenderMarchingCubes", &m_renderMarchingCubes);
    ImGui::Checkbox("RenderWired", &m_renderWired);
    ImGui::Checkbox("RenderRaycasting", &m_renderRayCasting);

    ImGui::Text("%d %.2f%%", m_numActiveParticles.m_cpu[0],
                100.0f * m_numActiveParticles.m_cpu[0] /
                    m_particles.m_cpu.size());
    ImGui::SliderInt("NumNewPts", &m_fluidConsts.m_cpu.numNewParticles, 0,
                     1024 * 4);

    if (ImGui::TreeNode("Post Processing")) {
        int flag = 0;
        flag += ImGui::SliderFloat(
            "Bloom Strength",
            &m_postProcess.m_combineFilter.m_constData.strength, 0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Exposure", &m_postProcess.m_combineFilter.m_constData.option1,
            0.0f, 10.0f);
        flag += ImGui::SliderFloat(
            "Motion Blur", &m_postProcess.m_combineFilter.m_constData.option3,
            0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Gamma", &m_postProcess.m_combineFilter.m_constData.option2, 0.1f,
            5.0f);
        // 편의상 사용자 입력이 인식되면 바로 GPU 버퍼를 업데이트
        if (flag) {
            m_postProcess.m_combineFilter.UpdateConstantBuffers(m_context);
        }
        ImGui::TreePop();
    }
}

} // namespace hlab