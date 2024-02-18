#include "Ex2001_GamePlay.h"

#include "BillboardModel.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "OceanModel.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex2001_GamePlay::Ex2001_GamePlay() : AppBase() {}

bool Ex2001_GamePlay::InitScene() {
     
    AppBase::m_globalConstsCPU.strengthIBL = 0.1f;
    AppBase::m_globalConstsCPU.lodBias = 0.0f;

    AppBase::m_camera.Reset(Vector3(1.60851f, 0.409084f, 0.560064f), -1.65915f,
                             0.0654498f);

    //AppBase::InitCubemaps(
    //    L"../Assets/Textures/Cubemaps/HDRI/", L"clear_pureskyEnvHDR.dds",
    //    L"clear_pureskySpecularHDR.dds", L"clear_pureskyDiffuseHDR.dds",
    //    L"clear_pureskyBrdf.dds");

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

        auto ground = make_shared<Model>(m_device, m_context, vector{mesh});
        ground->m_materialConsts.GetCpu().albedoFactor = Vector3(0.2f);
        ground->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        ground->m_materialConsts.GetCpu().metallicFactor = 0.5f;
        ground->m_materialConsts.GetCpu().roughnessFactor = 0.3f;
        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                               Matrix::CreateTranslation(position));
         
        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        // m_mirror = ground; // 바닥에 거울처럼 반사 구현

        m_basicList.push_back(ground); // 거울은 리스트에 등록 X
    }

    // Main Object
    {
        vector<string> clipNames = {"FightingIdleOnMichelle2.fbx",
                                    "Fireball.fbx"};
        string path = "../Assets/Characters/Mixamo/";

        AnimationData aniData;

        auto [meshes, _] =
            GeometryGenerator::ReadAnimationFromFile(path, "character.fbx");

        for (auto &name : clipNames) {
            auto [_, ani] =
                GeometryGenerator::ReadAnimationFromFile(path, name);

            if (aniData.clips.empty()) {
                aniData = ani;
            } else {
                aniData.clips.push_back(ani.clips.front());
            }
        }

        Vector3 center(0.0f, 0.1f, 1.0f);
        m_character =
            make_shared<SkinnedMeshModel>(m_device, m_context, meshes, aniData);
        m_character->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        m_character->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
        m_character->m_materialConsts.GetCpu().metallicFactor = 0.0f;
        m_character->UpdateWorldRow(Matrix::CreateScale(0.2f) *
                                    Matrix::CreateTranslation(center));

        m_basicList.push_back(m_character); // 리스트에 등록
        m_pickedModel = m_character;
    }

    InitPhysics(true);

    return true;
}

void Ex2001_GamePlay::CreateStack(const PxTransform &t, int numStacks,
                                  int numWidth, PxReal halfExtent) {

    vector<MeshData> box = {GeometryGenerator::MakeBox(halfExtent)};

    PxShape *shape = gPhysics->createShape(
        PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);

    for (int i = 0; i < numStacks; i++) {
        for (int j = 0; j < numWidth - i; j++) {
            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(numWidth - i),
                                       PxReal(i * 2 + 1), 0) *
                                halfExtent);
            PxRigidDynamic *body =
                gPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
            gScene->addActor(*body);

            auto m_newObj = std::make_shared<Model>(
                m_device, m_context, box); // <- 우리 렌더러에 추가
            m_newObj->m_materialConsts.GetCpu().albedoFactor = Vector3(0.8f);
            AppBase::m_basicList.push_back(m_newObj);
            this->m_objects.push_back(m_newObj);
        }
    }
    shape->release();
}

void Ex2001_GamePlay::InitPhysics(bool interactive) {
    gFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport *transport =
        PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation,
                               PxTolerancesScale(), true, gPvd);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,
                                   true);
    }
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic *groundPlane =
        PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);
     
    for (PxU32 i = 0; i < 5; i++)
        CreateStack(PxTransform(PxVec3(0, 0, stackZ -= 15.0f)), 8, 20, 2.5f);
}

PxRigidDynamic *Ex2001_GamePlay::CreateDynamic(const PxTransform &t,
                                               const PxGeometry &geometry,
                                               const PxVec3 &velocity) {
      
    m_fireball = std::make_shared<BillboardModel>();
     //m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
     //                       1.0f, L"GameExplosionPS.hlsl");
    Vector3 dir(velocity.x, velocity.y, velocity.z);
    dir.Normalize();
    m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
    m_fireball->m_castShadow = false;
    m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
                           0.2f, Graphics::volumetricFirePS);

    AppBase::m_basicList.push_back(m_fireball);
    this->m_objects.push_back(m_fireball);
     
    PxRigidDynamic *dynamic =
        PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    gScene->addActor(*dynamic);
    return dynamic;
}
 
void Ex2001_GamePlay::UpdateLights(float dt) { AppBase::UpdateLights(dt); }

void Ex2001_GamePlay::Update(float dt) {

    AppBase::Update(dt);

    static int frameCount = 0;
    static int state = 0;

    // TODO:
    if (state == 0) {
        if (AppBase::m_keyPressed[VK_SPACE]) {
            state = 1;
            frameCount = 0;
        }
    } else if (state == 1) {
        if (frameCount ==
            m_character->m_aniData.clips[1].keys[0].size()) {
            frameCount = 0;
            state = 0;

        }
        if (frameCount == 115) {
            Vector3 handPos = (m_character->m_worldRow).Translation();
            Vector4 offset = Vector4::Transform(
                Vector4(0.0f, 0.0f, -0.1f, 0.0f),
                m_character->m_worldRow *
                    m_character->m_aniData.accumulatedRootTransform);
            handPos += Vector3(offset.x, offset.y, offset.z);

            Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
            dir = Vector4::Transform(
                dir, m_character->m_worldRow *
                         m_character->m_aniData.accumulatedRootTransform);
            dir.Normalize();
            dir *= 1.5f / m_simToRenderScale;
            CreateDynamic(PxTransform(PxVec3(handPos.x, handPos.y, handPos.z) /
                                      m_simToRenderScale),
                          PxSphereGeometry(5), PxVec3(dir.x,dir.y,dir.z));
        }
    } 
     
    m_character->UpdateAnimation(m_context, state, frameCount);

    frameCount += 1;

    // 이하 물리엔진 관련

    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);
     
    // PxGeometryType::eBOX: , case PxGeometryType::eSPHERE:

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                         PxActorTypeFlag::eRIGID_STATIC);
    std::vector<PxRigidActor *> actors(nbActors);
    gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                          PxActorTypeFlag::eRIGID_STATIC,
                      reinterpret_cast<PxActor **>(&actors[0]), nbActors);
     
    PxShape *shapes[MAX_NUM_ACTOR_SHAPES];

    int count = 0;

    for (PxU32 i = 0; i < nbActors; i++) {
        const PxU32 nbShapes = actors[i]->getNbShapes();
        PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
        actors[i]->getShapes(shapes, nbShapes);
        for (PxU32 j = 0; j < nbShapes; j++) {
            const PxMat44 shapePose(
                PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
             
            if (actors[i]->is<PxRigidDynamic>()) {

                // bool speeping = actors[i]->is<PxRigidDynamic>() &&
                //                 actors[i]->is<PxRigidDynamic>()->isSleeping();
                  
                m_objects[count]->UpdateWorldRow(
                    Matrix(shapePose.front()) *
                    Matrix::CreateScale(
                        m_simToRenderScale) // PhysX to Render 스케일
                );
                m_objects[count]->UpdateConstantBuffers(m_device, m_context);

                count++;
            }
        }
    }

    /* PxContactPair 추출 하면 내 캐릭터가 어디에 닿았는지 찾을 수 있음
    * void onContact(const PxContactPairHeader& pairHeader, const PxContactPair*
    pairs, PxU32 nbPairs)
    {
        PX_UNUSED((pairHeader));
        std::vector<PxContactPairPoint> contactPoints;

        for(PxU32 i=0;i<nbPairs;i++)
        {
            PxU32 contactCount = pairs[i].contactCount;
            if(contactCount)
            {
                contactPoints.resize(contactCount);
                pairs[i].extractContacts(&contactPoints[0], contactCount);

                for(PxU32 j=0;j<contactCount;j++)
                {
                    gContactPositions.push_back(contactPoints[j].position);
                    gContactImpulses.push_back(contactPoints[j].impulse);
                }
            }
        }
    }
    */
}

void Ex2001_GamePlay::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void Ex2001_GamePlay::UpdateGUI() {
    AppBase::UpdateGUI();
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
    if (m_mirror && ImGui::TreeNode("Mirror")) {

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
