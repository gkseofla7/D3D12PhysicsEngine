#include "Ex1605_SmokeCpu.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1605_SmokeCpu::Ex1605_SmokeCpu() : AppBase() {}

bool Ex1605_SmokeCpu::InitScene() {

    cout << "Ex1605_SmokeCpu::InitScene()" << endl;

    AppBase::m_camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);
    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

    // Volume 쉐이더는 별도로 초기화
    AppBase::InitScene();

    Graphics::InitVolumeShaders(m_device);

    m_globalConstsCPU.strengthIBL = 0.01f;

    m_volumeConsts.m_cpu.densityAbsorption = 2.0f;
    m_volumeConsts.Initialize(m_device);

    m_fluid.Initialize(64, 64, 64);

    Vector3 center(0.0f);
    m_volumeModel = make_shared<Model>(
        m_device, m_context, vector{GeometryGenerator::MakeBox(1.0f)});
    m_volumeModel->UpdateWorldRow(Matrix::CreateScale(1.5f) *
                                  Matrix::CreateTranslation(center));
    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    m_volumeModel->m_meshes.front()->densityTex.Initialize(
        m_device, m_fluid.m_grid.m_res[0], m_fluid.m_grid.m_res[1],
        m_fluid.m_grid.m_res[2], DXGI_FORMAT_R32_FLOAT, {/* Empty */});

    return true;
}

void Ex1605_SmokeCpu::Update(float dt) {

    AppBase::Update(dt);

    m_volumeModel->UpdateConstantBuffers(m_device, m_context);

    if (!AppBase::m_pauseAnimation) {
        m_fluid.Update(dt);
        m_volumeModel->m_meshes.front()->densityTex.Upload(m_device, m_context,
                                                           m_fluid.m_density);
    }
}

void Ex1605_SmokeCpu::Render() {

    AppBase::Render();

    // Bounding Sphere Wire Frame으로 그리기

    // AppBase::SetPipelineState(Graphics::defaultSolidPSO);

    // MeshConstants를 PixelShader에도 넣어줘야함

    AppBase::SetPipelineState(Graphics::volumeSmokePSO);
    m_context->PSSetConstantBuffers(3, 1, m_volumeConsts.GetAddressOf());
    m_volumeModel->Render(m_context);

    // AppBase::SetPipelineState(Graphics::defaultWirePSO);
    // m_volumeModel->Render(m_context);
    // m_boundingSphereModel->Render(m_context);

    AppBase::SetPipelineState(Graphics::boundingBoxPSO);
    m_volumeModel->RenderWireBoundingBox(m_context);

    AppBase::PostRender();
}

void Ex1605_SmokeCpu::UpdateGUI() { AppBase::UpdateGUI(); }

} // namespace hlab