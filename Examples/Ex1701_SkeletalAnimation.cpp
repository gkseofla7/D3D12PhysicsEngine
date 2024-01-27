#include "Ex1701_SkeletalAnimation.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "SkinnedMeshModel.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1701_SkeletalAnimation::Ex1701_SkeletalAnimation() : AppBase() {}

bool Ex1701_SkeletalAnimation::InitScene() {
      
    AppBase::m_globalConstsCPU.strengthIBL = 1.0f;
    m_walkingState = WalkingAnimState::Idle;
    m_specialState = SpecialAnimState::None;
    AppBase::m_camera.Reset(Vector3(3.74966f, 5.03645f, -2.54918f), -0.819048f,
                            0.741502f);

    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

    AppBase::InitScene();

    // 조명 설정
    {
        // 조명 0은 고정
        m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[0].position = Vector3(0.0f, 2.0f, 2.0f);
        m_globalConstsCPU.lights[0].direction = Vector3(0.2f, -1.0f, 0.0f);
        m_globalConstsCPU.lights[0].spotPower = 0.0f;
        m_globalConstsCPU.lights[0].radius = 0.1f;
        m_globalConstsCPU.lights[0].type = LIGHT_POINT | LIGHT_SHADOW;

        m_globalConstsCPU.lights[1].type = LIGHT_OFF;
        m_globalConstsCPU.lights[2].type = LIGHT_OFF;
    }
     
    // 바닥(거울)
    {
        // https://freepbr.com/materials/stringy-marble-pbr/
        auto mesh = GeometryGenerator::MakeSquare(5.0);
        string path = "../Assets/Textures/PBR/metalfloor/";
        mesh.albedoTextureFilename = path + "MetalPlates003_1K-PNG_Color.png";
        mesh.emissiveTextureFilename = "";
        mesh.aoTextureFilename = ""; // path + "stringy_marble_ao.png";
        mesh.metallicTextureFilename = path + "MetalPlates003_1K-PNG_Metalness.png";
        mesh.normalTextureFilename = path + "MetalPlates003_1K-PNG_NormalDX.png";
        mesh.roughnessTextureFilename = path + "MetalPlates003_1K-PNG_Roughness.png";

        m_ground = make_shared<Model>(m_device, m_context, vector{mesh});
        m_ground->m_materialConsts.GetCpu().albedoFactor = Vector3(0.7f);
        m_ground->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        m_ground->m_materialConsts.GetCpu().metallicFactor = 0.5f;
        m_ground->m_materialConsts.GetCpu().roughnessFactor = 0.3f;

        Vector3 position = Vector3(0.0f, 0.0f, 2.0f);
        m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));
        m_ground->m_castShadow = false; // 바닥은 그림자 만들기 생략

        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        m_mirror = m_ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X
    }
     
    // Main Object
    { 
        AnimStateToAnim[WalkingAnimState::Idle] = "Neutral Idle.fbx";
        AnimStateToAnim[WalkingAnimState::IdleToWalk] = "Start Walking.fbx";
        AnimStateToAnim[WalkingAnimState::WalkingBackward] =
            "Walking Backwards.fbx";
        AnimStateToAnim[WalkingAnimState::Walking] = "Strut Walking.fbx";
        AnimStateToAnim[WalkingAnimState::WalkingToIdle] = "Stop Walking.fbx";
        AnimStateToAnim[WalkingAnimState::CombatStateStart] =
            "Standing Idle To Fight Idle.fbx";
        AnimStateToAnim[WalkingAnimState::CombatState] =
            "Bouncing Fight Idle.fbx";
        AnimStateToAnim[SpecialAnimState::Punching] = "Punching Bag.fbx";
        AnimStateToAnim[SpecialAnimState::Hook] = "Hook Punch.fbx";
        string path = "../Assets/Characters/PunchFighter/";
         
        AnimationData aniData;
        auto [meshes, _] =
            GeometryGenerator::ReadAnimationFromFile(path, "Punching Bag.fbx");
         
        for (auto &Pair : AnimStateToAnim) {
            const string &name = Pair.second;
            auto [_, ani] =
                GeometryGenerator::ReadAnimationFromFile(path, name);

            if (aniData.clips.empty()) {
                aniData = ani;
            } else {
                aniData.clips.push_back(ani.clips.front());
            }
        }
          
        Vector3 center(0.0f, 0.5f, 2.0f);
        m_character =
            make_shared<SkinnedMeshModel>(m_device, m_context, meshes, aniData);
        m_character->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        m_character->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
        m_character->m_materialConsts.GetCpu().metallicFactor = 0.0f;
        m_character->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                    Matrix::CreateTranslation(center));

        m_basicList.push_back(m_character); // 리스트에 등록
        m_pickedModel = m_character;
    }

    return true;
}

void Ex1701_SkeletalAnimation::UpdateLights(float dt) {
    AppBase::UpdateLights(dt);
}

void Ex1701_SkeletalAnimation::Update(float dt) {
    AppBase::Update(dt);

    static int frameCount = 0;
    static float accumulateTime = 0.f;

    accumulateTime += dt;
    
    bool IsPlayingAT = false;
    {
        // States
        // 0: idle
        // 1: idle to walk
        // 2: walk forward
        // 3: walk to stop
        // 4: dance

        // static int state = 0;

        // TODO:
        // 간단한 모션 그래프 구현
        // "Motion Graphs" by Kovar et al. ACM SIGGRAPH 2002

        // 힌트:
        // if (AppBase::m_keyPressed[VK_UP]) , VK_RIGHT, VK_LEFT, VK_UP
        // if (frameCount == m_character->m_aniData.clips[state].keys[0].size())
        // m_character->m_aniData.accumulatedRootTransform =
        //     Matrix::CreateRotationY(3.141592f * 60.0f / 180.0f * dt) *
        //     m_character->m_aniData.accumulatedRootTransform

        // 주의: frameCount = 0;
        // if (state == 0) { // 정지 상태
        //    // TODO:
        //    if (AppBase::m_keyPressed[VK_UP]) {
        //        state = 1;
        //        frameCount = 0;
        //    }
        //}

        // if (state == 1) {
        //         if (frameCount ==
        //         m_character->m_aniData.clips[state].keys[0].size()) { state =
        //         2; frameCount = 0;
        //     }
        // }
        // if (state == 2) {
        //     if (frameCount ==
        //     m_character->m_aniData.clips[state].keys[0].size()) {
        //         frameCount = 0;
        //     }
        //     if (AppBase::m_keyPressed[VK_UP] == false) {
        //         state = 3;
        //         frameCount = 0;
        //     }
        //     bool rightPressed = AppBase::m_keyPressed[VK_RIGHT];
        //     bool leftPressed = AppBase::m_keyPressed[VK_LEFT];

        //    if ((rightPressed || leftPressed) && rightPressed != leftPressed)
        //    {
        //        float sign = leftPressed == true ? 1.0f : -1.0f;
        //        m_character->m_aniData.accumulatedRootTransform =
        //            Matrix::CreateRotationY(sign *
        //                                    (3.141592f * 60.0f / 180.0f * dt))
        //                                    *
        //            m_character->m_aniData.accumulatedRootTransform;
        //    }
        //}
        // if (state == 3) {
        //    if (frameCount ==
        //    m_character->m_aniData.clips[state].keys[0].size()) {
        //        state = 4;
        //        frameCount = 0;
        //    }
        //}
        // if (state == 4) {
        //    if (frameCount ==
        //    m_character->m_aniData.clips[state].keys[0].size()) {
        //        state = 0;
        //        frameCount = 0;
        //    }
        //}
    }
    switch (m_walkingState) {
    case WalkingAnimState::Idle:
        if (frameCount ==
            m_character->m_aniData.clips[m_walkingState].keys[0].size()) {
            frameCount = 0;
        }
        if (AppBase::m_keyPressed[VK_UP]) {
            m_walkingState = WalkingAnimState::IdleToWalk;
            frameCount = 0;
            accumulateTime = 0.f;
        }
        break;
    case WalkingAnimState::IdleToWalk:
        IsPlayingAT = true;
        if (frameCount ==
            m_character->m_aniData.clips[m_walkingState].keys[0].size()) {
            m_walkingState = WalkingAnimState::Walking;
            frameCount = 0;
            accumulateTime = 0.f;
        }
        break;
    case WalkingAnimState::Walking:
        if (frameCount ==
            m_character->m_aniData.clips[m_walkingState].keys[0].size()) {
            frameCount = 0;
            accumulateTime = 0.f;
        }
        if (AppBase::m_keyPressed[VK_UP] == false) {
            m_walkingState = WalkingAnimState::WalkingToIdle;
            frameCount = 0;
            accumulateTime = 0.f;
        } else {
            bool rightPressed = AppBase::m_keyPressed[VK_RIGHT];
            bool leftPressed = AppBase::m_keyPressed[VK_LEFT];

            if ((rightPressed || leftPressed) && rightPressed != leftPressed) {
                float sign = rightPressed == true ? 1.0f : -1.0f;
                m_character->m_aniData.accumulatedRootTransform =
                    Matrix::CreateRotationY(sign *
                                            (3.141592f * 60.0f / 180.0f * dt)) *
                    m_character->m_aniData.accumulatedRootTransform;
            } 
            m_character->m_aniData.accumulatedRootTransform =
                Matrix::CreateTranslation(Vector3(0., 0., -dt * 300.0f)) *
                m_character->m_aniData.accumulatedRootTransform;
        }
        break;
    case WalkingAnimState::WalkingToIdle:
        IsPlayingAT = true;
        if (frameCount ==
            m_character->m_aniData.clips[m_walkingState].keys[0].size()) {
            m_walkingState = WalkingAnimState::Idle;
            frameCount = 0;
            accumulateTime = 0.f;
        }
        break;
    case WalkingAnimState::CombatState:
        if (frameCount ==
            m_character->m_aniData.clips[m_walkingState].keys[0].size()) {
            frameCount = 0;
        }
        if (accumulateTime >= 2.0f) {
            m_walkingState = WalkingAnimState::Idle;
            frameCount = 0;
            accumulateTime = 0.f;
        }
        break;
    }
    m_character->UpdateAnimation(m_context, (int)m_walkingState, frameCount, 0);
    if (IsPlayingAT == false) {
        static int specialFrameCount = 0;
        if (m_playingSpecialAT == false) {
            if (AppBase::m_keyPressed[VK_SPACE] &&
                AppBase::m_keyPressed[VK_SHIFT] == false) {
                m_specialState = SpecialAnimState::Punching;
                specialFrameCount = 0;
                m_playingSpecialAT = true;
            }
            if (AppBase::m_keyPressed[VK_SPACE] &&
                AppBase::m_keyPressed[VK_SHIFT] == true) {
                m_specialState = SpecialAnimState::Hook;
                specialFrameCount = 0;
                m_playingSpecialAT = true;
            }
        }
        if (m_playingSpecialAT == true) {
            if (specialFrameCount ==
                m_character->m_aniData.clips[m_specialState].keys[0].size()) {
                specialFrameCount = 0;
                m_playingSpecialAT = false;
                m_specialState = SpecialAnimState::None;
            } else {
                m_character->UpdateAnimation(m_context, (int)m_specialState,
                                             specialFrameCount, 1);
            }
        } else {
            // m_character->UpdateAnimation(m_context, (int)m_walkingState,
            // frameCount, 2);
        }
        specialFrameCount += 1;
    }
    // cout << (int)m_state << endl;
    frameCount += 1;
    m_character->UpdateVelocity(dt);
}
void Ex1701_SkeletalAnimation::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void Ex1701_SkeletalAnimation::UpdateGUI() {

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
