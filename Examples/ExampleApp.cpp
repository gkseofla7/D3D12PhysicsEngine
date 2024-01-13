#include "ExampleApp.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

ExampleApp::ExampleApp() : AppBase() {}

bool ExampleApp::InitScene() {

    AppBase::m_camera.Reset(Vector3(-0.112852f, 0.307729f, -0.542159f),
                            0.0589047f, 0.14399f);
    AppBase::m_globalConstsCPU.strengthIBL = 0.1f;

    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

    AppBase::InitScene();

    // 바닥(거울)
    {
        // https://freepbr.com/materials/stringy-marble-pbr/
        auto mesh = GeometryGenerator::MakeSquare(5.0, {10.0f, 10.0f});
        string path = "../Assets/Textures/PBR/stringy-marble-ue/";
        mesh.albedoTextureFilename = path + "stringy_marble_albedo.png";
        mesh.emissiveTextureFilename = "";
        mesh.aoTextureFilename = path + "stringy_marble_ao.png";
        mesh.metallicTextureFilename = path + "stringy_marble_Metallic.png";
        mesh.normalTextureFilename = path + "stringy_marble_Normal-dx.png";
        mesh.roughnessTextureFilename = path + "stringy_marble_Roughness.png";

        m_ground = make_shared<Model>(m_device, m_context, vector{mesh});
        m_ground->m_materialConsts.GetCpu().albedoFactor = Vector3(0.7f);
        m_ground->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        m_ground->m_materialConsts.GetCpu().metallicFactor = 0.5f;
        m_ground->m_materialConsts.GetCpu().roughnessFactor = 0.3f;
        m_ground->m_name = "Ground";

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));

        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        m_mirror = m_ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X
    }

    // Main Object
    {
        // auto meshes = GeometryGenerator::ReadFromFile(
        //     "../Assets/Models/DamagedHelmet/", "DamagedHelmet.gltf");

        // auto meshes = GeometryGenerator::ReadFromFile(
        //     "../Assets/Models/medieval_vagrant_knights/", "scene.gltf",
        //     true);

        // 컴퓨터가 느릴 때는 간단한 물체로 테스트 하세요.
        vector<MeshData> meshes = {GeometryGenerator::MakeSphere(0.4f, 50, 50)};

        // string path = "../Assets/Characters/armored-female-future-soldier/";
        // auto meshes = GeometryGenerator::ReadFromFile(path,
        // "angel_armor.fbx"); meshes[0].albedoTextureFilename = path +
        // "/angel_armor_albedo.jpg"; meshes[0].emissiveTextureFilename = path +
        // "/angel_armor_e.jpg"; meshes[0].metallicTextureFilename = path +
        // "/angel_armor_metalness.jpg"; meshes[0].normalTextureFilename = path
        // + "/angel_armor_normal.jpg"; meshes[0].roughnessTextureFilename =
        //     path + "/angel_armor_roughness.jpg";

        Vector3 center(0.0f, -0.05f, 2.0f);
        auto newModel = make_shared<Model>(m_device, m_context, meshes);
        newModel->m_materialConsts.GetCpu().invertNormalMapY =
            true; // GLTF는 true로
        newModel->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        newModel->m_materialConsts.GetCpu().roughnessFactor = 0.3f;
        newModel->m_materialConsts.GetCpu().metallicFactor = 0.8f;
        newModel->UpdateWorldRow(Matrix::CreateTranslation(center));
        newModel->m_isPickable = true; // 마우스로 선택/이동 가능
        newModel->m_name = "MainSphere";

        m_basicList.push_back(newModel); // 리스트에 등록
    }

    // 추가 물체1
    {
        MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
        Vector3 center(0.5f, 0.5f, 2.0f);
        auto newModel = make_shared<Model>(m_device, m_context, vector{mesh});
        newModel->UpdateWorldRow(Matrix::CreateTranslation(center));
        newModel->m_materialConsts.GetCpu().albedoFactor =
            Vector3(0.1f, 0.1f, 1.0f);
        newModel->m_materialConsts.GetCpu().roughnessFactor = 0.2f;
        newModel->m_materialConsts.GetCpu().metallicFactor = 0.6f;
        newModel->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        newModel->UpdateConstantBuffers(m_device, m_context);
        newModel->m_isPickable = true; // 마우스로 선택/이동 가능
        newModel->m_name = "SecondSphere";
        m_basicList.push_back(newModel);
    }

    // 추가 물체2
    {
        MeshData mesh = GeometryGenerator::MakeBox(0.3f);
        Vector3 center(0.0f, 0.5f, 2.5f);
        auto newModel = make_shared<Model>(m_device, m_context, vector{mesh});
        newModel->UpdateWorldRow(Matrix::CreateTranslation(center));
        newModel->m_materialConsts.GetCpu().albedoFactor =
            Vector3(1.0f, 0.2f, 0.2f);
        newModel->m_materialConsts.GetCpu().roughnessFactor = 0.5f;
        newModel->m_materialConsts.GetCpu().metallicFactor = 0.9f;
        newModel->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        newModel->UpdateConstantBuffers(m_device, m_context);
        newModel->m_isPickable = true; // 마우스로 선택/이동 가능
        newModel->m_name = "Box";
        m_basicList.push_back(newModel);
    }

    return true;
}

void ExampleApp::UpdateLights(float dt) { AppBase::UpdateLights(dt); }

void ExampleApp::Update(float dt) { AppBase::Update(dt); }

void ExampleApp::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void ExampleApp::UpdateGUI() {

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_camera.m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        ImGui::Checkbox("DrawOBB", &m_drawOBB);
        ImGui::Checkbox("DrawBSphere", &m_drawBS);
        if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
            CreateBuffers();
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Skybox")) {
        ImGui::SliderFloat("Strength", &m_globalConstsCPU.strengthIBL, 0.0f,
                           0.5f);
        ImGui::RadioButton("Env", &m_globalConstsCPU.textureToDraw, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Specular", &m_globalConstsCPU.textureToDraw, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Irradiance", &m_globalConstsCPU.textureToDraw, 2);
        ImGui::SliderFloat("EnvLodBias", &m_globalConstsCPU.envLodBias, 0.0f,
                           10.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Effects")) {
        int flag = 0;
        flag += ImGui::RadioButton("Render", &m_postEffectsConstsCPU.mode, 1);
        ImGui::SameLine();
        flag += ImGui::RadioButton("Depth", &m_postEffectsConstsCPU.mode, 2);
        flag += ImGui::SliderFloat(
            "DepthScale", &m_postEffectsConstsCPU.depthScale, 0.0, 1.0);
        flag += ImGui::SliderFloat("Fog", &m_postEffectsConstsCPU.fogStrength,
                                   0.0, 10.0);

        if (flag)
            D3D11Utils::UpdateBuffer(m_context, m_postEffectsConstsCPU,
                                     m_postEffectsConstsGPU);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Post Processing")) {
        int flag = 0;
        flag += ImGui::SliderFloat(
            "Bloom Strength",
            &m_postProcess.m_combineFilter.m_constData.strength, 0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Exposure", &m_postProcess.m_combineFilter.m_constData.option1,
            0.0f, 10.0f);
        flag += ImGui::SliderFloat(
            "Gamma", &m_postProcess.m_combineFilter.m_constData.option2, 0.1f,
            5.0f);
        // 편의상 사용자 입력이 인식되면 바로 GPU 버퍼를 업데이트
        if (flag) {
            m_postProcess.m_combineFilter.UpdateConstantBuffers(m_context);
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Mirror")) {

        ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
        const float blendColor[4] = {m_mirrorAlpha, m_mirrorAlpha,
                                     m_mirrorAlpha, 1.0f};
        if (m_drawAsWire)
            Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
        else
            Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

        ImGui::SliderFloat("Metallic",
                           &m_mirror->m_materialConsts.GetCpu().metallicFactor,
                           0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",
                           &m_mirror->m_materialConsts.GetCpu().roughnessFactor,
                           0.0f, 1.0f);

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light")) {
        // ImGui::SliderFloat3("Position",
        // &m_globalConstsCPU.lights[0].position.x,
        //                     -5.0f, 5.0f);
        ImGui::SliderFloat("Halo Radius",
                           &m_globalConstsCPU.lights[1].haloRadius, 0.0f, 2.0f);
        ImGui::SliderFloat("Halo Strength",
                           &m_globalConstsCPU.lights[1].haloStrength, 0.0f,
                           1.0f);
        ImGui::SliderFloat("Radius", &m_globalConstsCPU.lights[1].radius, 0.0f,
                           0.5f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material")) {
        ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias, 0.0f, 10.0f);

        int flag = 0;

        if (m_pickedModel) {
            flag += ImGui::SliderFloat(
                "Metallic",
                &m_pickedModel->m_materialConsts.GetCpu().metallicFactor, 0.0f,
                1.0f);
            flag += ImGui::SliderFloat(
                "Roughness",
                &m_pickedModel->m_materialConsts.GetCpu().roughnessFactor, 0.0f,
                1.0f);
            flag += ImGui::CheckboxFlags(
                "AlbedoTexture",
                &m_pickedModel->m_materialConsts.GetCpu().useAlbedoMap, 1);
            flag += ImGui::CheckboxFlags(
                "EmissiveTexture",
                &m_pickedModel->m_materialConsts.GetCpu().useEmissiveMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use NormalMapping",
                &m_pickedModel->m_materialConsts.GetCpu().useNormalMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use AO", &m_pickedModel->m_materialConsts.GetCpu().useAOMap,
                1);
            flag += ImGui::CheckboxFlags(
                "Use HeightMapping",
                &m_pickedModel->m_meshConsts.GetCpu().useHeightMap, 1);
            flag += ImGui::SliderFloat(
                "HeightScale",
                &m_pickedModel->m_meshConsts.GetCpu().heightScale, 0.0f, 0.1f);
            flag += ImGui::CheckboxFlags(
                "Use MetallicMap",
                &m_pickedModel->m_materialConsts.GetCpu().useMetallicMap, 1);
            flag += ImGui::CheckboxFlags(
                "Use RoughnessMap",
                &m_pickedModel->m_materialConsts.GetCpu().useRoughnessMap, 1);
            if (flag) {
                m_pickedModel->UpdateConstantBuffers(m_device, m_context);
            }
            ImGui::Checkbox("Draw Normals", &m_pickedModel->m_drawNormals);
        }

        ImGui::TreePop();
    }
}

} // namespace hlab
