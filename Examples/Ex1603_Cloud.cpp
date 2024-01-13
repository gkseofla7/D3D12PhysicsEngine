#include "Ex1603_Cloud.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "Noise.h"

#include <glm/glm.hpp>
#include <glm/simd/common.h>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace glm;

Ex1603_Cloud::Ex1603_Cloud() : AppBase() {}

bool Ex1603_Cloud::InitScene() {

    cout << "Ex1603_Cloud::InitScene()" << endl;

    AppBase::m_camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);

    // https://polyhaven.com/a/syferfontein_18d_clear_puresky
    AppBase::InitCubemaps(
        L"../Assets/Textures/Cubemaps/HDRI/", L"clear_pureskyEnvHDR.dds",
        L"clear_pureskySpecularHDR.dds", L"clear_pureskyDiffuseHDR.dds",
        L"clear_pureskyBrdf.dds");

    AppBase::InitScene();

    // Volume 쉐이더는 별도로 초기화
    Graphics::InitVolumeShaders(m_device);

    m_globalConstsCPU.strengthIBL = 0.3f;

    Vector3 center(0.0f);
    m_volumeModel = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeBox(1.0f)});
    m_volumeModel->UpdateWorldRow(Matrix::CreateScale(1.5f) *
                                  Matrix::CreateTranslation(center));
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    m_volumeModel->m_meshes.front()->densityTex.Initialize(
        m_device, m_volumeWidth, m_volumeHeight, m_volumeDepth,
        DXGI_FORMAT_R16_FLOAT, {});
    m_volumeModel->m_meshes.front()->lightingTex.Initialize(
        m_device, m_lightWidth, m_lightHeight, m_lightDepth,
        DXGI_FORMAT_R16_FLOAT, {});

    // Generate cloud volume data
    m_volumeConstsCpu.uvwOffset = Vector3(0.0f);
    D3D11Utils::CreateConstBuffer(m_device, m_volumeConstsCpu,
                                  m_volumeConstsGpu);
    D3D11Utils::CreateComputeShader(m_device, L"CloudDensityCS.hlsl",
                                    m_cloudDensityCS);
    D3D11Utils::CreateComputeShader(m_device, L"CloudLightingCS.hlsl",
                                    m_cloudLightingCS);

    m_context->CSSetConstantBuffers(0, 1, m_volumeConstsGpu.GetAddressOf());
    m_context->CSSetUnorderedAccessViews(
        0, 1, m_volumeModel->m_meshes.front()->densityTex.GetAddressOfUAV(),
        NULL);
    m_context->CSSetShader(m_cloudDensityCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_volumeWidth / 16.0f)),
                        UINT(ceil(m_volumeHeight / 16.0f)),
                        UINT(ceil(m_volumeDepth / 4.0f)));
    AppBase::ComputeShaderBarrier();

    m_context->CSSetShaderResources(
        0, 1, m_volumeModel->m_meshes.front()->densityTex.GetAddressOfSRV());
    m_context->CSSetUnorderedAccessViews(
        0, 1, m_volumeModel->m_meshes.front()->lightingTex.GetAddressOfUAV(),
        NULL);
    m_context->CSSetShader(m_cloudLightingCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_lightWidth / 16.0f)),
                        UINT(ceil(m_lightHeight / 16.0f)),
                        UINT(ceil(m_lightDepth / 4.0f)));
    AppBase::ComputeShaderBarrier();

    return true;
}

void Ex1603_Cloud::Update(float dt) {

    AppBase::Update(dt);

    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    static float offset = 0.0f;

    offset += 0.003f; // 애니메이션 효과

    m_volumeConstsCpu.uvwOffset.z = offset;
    D3D11Utils::UpdateBuffer(m_context, m_volumeConstsCpu, m_volumeConstsGpu);

    m_context->CSSetConstantBuffers(0, 1, m_volumeConstsGpu.GetAddressOf());
    m_context->CSSetUnorderedAccessViews(
        0, 1, m_volumeModel->m_meshes.front()->densityTex.GetAddressOfUAV(),
        NULL);
    m_context->CSSetShader(m_cloudDensityCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_volumeWidth / 16.0f)),
                        UINT(ceil(m_volumeHeight / 16.0f)),
                        UINT(ceil(m_volumeDepth / 4.0f)));
    AppBase::ComputeShaderBarrier();

    m_context->CSSetShaderResources(
        0, 1, m_volumeModel->m_meshes.front()->densityTex.GetAddressOfSRV());
    m_context->CSSetUnorderedAccessViews(
        0, 1, m_volumeModel->m_meshes.front()->lightingTex.GetAddressOfUAV(),
        NULL);
    m_context->CSSetShader(m_cloudLightingCS.Get(), 0, 0);
    m_context->Dispatch(UINT(ceil(m_lightWidth / 16.0f)),
                        UINT(ceil(m_lightHeight / 16.0f)),
                        UINT(ceil(m_lightDepth / 4.0f)));
    AppBase::ComputeShaderBarrier();
}

void Ex1603_Cloud::Render() {
    AppBase::Render();

    AppBase::SetPipelineState(Graphics::volumeSmokePSO);
    m_context->PSSetConstantBuffers(3, 1, m_volumeConstsGpu.GetAddressOf());
    m_volumeModel->Render(m_context);

    AppBase::PostRender();
}

void Ex1603_Cloud::UpdateGUI() {
    AppBase::UpdateGUI();

    static float lightAngle = 90.0f;
    if (ImGui::SliderFloat("LightAngle", &lightAngle, 0.0f, 180.0f)) {
        m_volumeConstsCpu.lightDir.x = glm::cos(glm::radians(lightAngle));
        m_volumeConstsCpu.lightDir.y = glm::sin(glm::radians(lightAngle));
    }

    ImGui::SliderFloat("LightAbsorption", &m_volumeConstsCpu.lightAbsorption,
                       0.0f, 100.0f);

    ImGui::SliderFloat("DensityAbsorption",
                       &m_volumeConstsCpu.densityAbsorption, 0.0f, 100.0f);

    static float lightScale = 40.0f;
    if (ImGui::SliderFloat("lightScale", &lightScale, 0.0f, 400.0f)) {
        m_volumeConstsCpu.lightColor = Vector3(1.0f) * lightScale;
    }

    ImGui::SliderFloat("Aniso", &m_volumeConstsCpu.aniso, 0.0f, 1.0f);
}

} // namespace hlab