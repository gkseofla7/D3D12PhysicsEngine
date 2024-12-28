//#include "DaerimGame.h"
//#include "AnimHelper.h"
//#include "Wizard.h"
//#include "DSkinnedMeshModel.h"
//#include "MeshLoadHelper.h"
//#include "ProjectileManager.h"
//#include "DaerimsEngineBase.h"
//
//#include "bullet/btBulletDynamicsCommon.h"
//#include "bullet/btBulletCollisionCommon.h"
//#include "bullet/BulletCollision/btBulletCollisionCommon.h"
//namespace dengine {
//
//using namespace std;
//using namespace DirectX;
//using namespace DirectX::SimpleMath;
//
//DaerimGame::DaerimGame() :Engine() {}
//
//bool DaerimGame::InitScene()
//{
//    AnimHelper::GetInstance().Initialize(m_device, m_context);
//    ProjectileManager::GetInstance().Initialize(this, m_device, m_context);
//
//    AppBase::m_globalConstsCPU.strengthIBL = 0.1f;
//    AppBase::m_globalConstsCPU.lodBias = 0.0f;
//
//    //카메라를 부착하는 기능도 필요하다.
//    AppBase::m_camera.Reset(Vector3(1.60851f, 0.409084f, 0.560064f), -1.65915f,
//        0.0654498f);
//
//    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
//        L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
//        L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");
//
//    AppBase::InitScene();
//    InitAnimation();
//    // 바닥(거울)
//    {
//
//        // https://freepbr.com/materials/stringy-marble-pbr/
//        auto mesh = GeometryGenerator::MakeSquare(5.0, { 10.0f, 10.0f });
//        string path = "../Assets/Textures/PBR/stringy-marble-ue/";
//        mesh.albedoTextureFilename = path + "stringy_marble_albedo.png";
//        mesh.emissiveTextureFilename = "";
//        mesh.aoTextureFilename = path + "stringy_marble_ao.png";
//        mesh.metallicTextureFilename = path + "stringy_marble_Metallic.png";
//        mesh.normalTextureFilename = path + "stringy_marble_Normal-dx.png";
//        mesh.roughnessTextureFilename = path + "stringy_marble_Roughness.png";
//
//        auto ground = make_shared<Model>(m_device, m_context, vector{ mesh });
//        ground->m_materialConsts.GetCpu().albedoFactor = Vector3(0.2f);
//        ground->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
//        ground->m_materialConsts.GetCpu().metallicFactor = 0.5f;
//        ground->m_materialConsts.GetCpu().roughnessFactor = 0.3f;
//        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
//        ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
//            Matrix::CreateTranslation(position));
//
//        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
//        // m_mirror = ground; // 바닥에 거울처럼 반사 구현
//
//        m_basicList.push_back(ground); // 거울은 리스트에 등록 X
//
//    }
//
//
//    // Main Object
//    {
//        //for (int i = 0; i < 10; i++) 
//        //{
//        //    for(int j = 0; j<10; j++)
//        //    {
//        //        string path = "../Assets/Characters/Mixamo/";
//        //        string characterName = "character.fbx";
//        //        Vector3 center(0.0f + 0.2f * i, 0.1f, 1.0f + 0.2f * j);
//        //        shared_ptr<DSkinnedMeshModel> wizardModel = make_shared<DSkinnedMeshModel>(m_device, m_context, path, characterName);
//        //        wizardModel->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
//        //        wizardModel->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
//        //        wizardModel->m_materialConsts.GetCpu().metallicFactor = 0.0f;
//        //        wizardModel->UpdateWorldRow(Matrix::CreateScale(0.2f) * Matrix::CreateRotationY(3.14f / 2 * 3) *
//        //            Matrix::CreateTranslation(center));
//        //        wizardModel->SetScale(0.2f);
//        //        shared_ptr<Wizard> wizardActor =
//        //            make_shared<Wizard>(m_device, m_context, wizardModel);
//        //        wizardActor->Initialize(m_device, m_context, wizardModel);
//        //        m_activateActor = wizardActor;
//        //        m_objectList.push_back(wizardActor);
//        //    }
//        //}
//
//        {
//            string path = "../Assets/Characters/Mixamo/";
//            string characterName = "character.fbx";
//            Vector3 center(0.5f, 0.1f, 1.0f);
//            shared_ptr<DSkinnedMeshModel> wizardModel = make_shared<DSkinnedMeshModel>(m_device, m_context, path, characterName);
//            wizardModel->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
//            wizardModel->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
//            wizardModel->m_materialConsts.GetCpu().metallicFactor = 0.0f;
//            wizardModel->UpdateWorldRow(Matrix::CreateScale(0.2f) *
//                Matrix::CreateTranslation(center));
//            wizardModel->SetScale(0.2f);
//            shared_ptr<Wizard> wizardActor =
//                make_shared<Wizard>(m_device, m_context, wizardModel);
//            wizardActor->Initialize(m_device, m_context, wizardModel);
//            m_objectList.push_back(wizardActor); // 리스트에 등록, 이거 왜..?
//        }
//
//        {
//            string path = "../Assets/Characters/Mixamo/";
//            string characterName = "character.fbx";
//            Vector3 center(0.5f + 1.0f, 0.1f, 1.0f);
//            shared_ptr<DSkinnedMeshModel> wizardModel = make_shared<DSkinnedMeshModel>(m_device, m_context, path, characterName);
//            wizardModel->m_materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
//            wizardModel->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
//            wizardModel->m_materialConsts.GetCpu().metallicFactor = 0.0f;
//            wizardModel->UpdateWorldRow(Matrix::CreateScale(0.2f) *
//                Matrix::CreateTranslation(center));
//            wizardModel->SetScale(0.2f);
//            shared_ptr<Wizard> wizardActor =
//                make_shared<Wizard>(m_device, m_context, wizardModel);
//            wizardActor->Initialize(m_device, m_context, wizardModel);
//            m_objectList.push_back(wizardActor); // 리스트에 등록, 이거 왜..?
//        }
//
//    }
//
//    InitPhysics(true);
//
//    return true;
//}
//void DaerimGame::InitAnimation()
//{
//    //Wizard
//    // TODO 테스트를 위해 하드 코딩, 동작 확인후 변경 예정
//
//}
//void DaerimGame::InitPhysics(bool interactive)
//{
//    DaerimsEngineBase::GetInstance().InitPhysEngine();
//    //m_dynamicsWorld->setGravity(btVector3(0,0,0));
//
//    ///create a few basic rigid bodies
//    // TODO. 두번째 파라미터 어디에..
//    btStaticPlaneShape* groundShape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), btScalar(1.0f));
//
//    //groundShape->initializePolyhedralFeatures();
//    //btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
//
//    m_collisionShapes.push_back(groundShape);
//
//    btTransform groundTransform;
//    groundTransform.setIdentity();
//    groundTransform.setOrigin(btVector3(0, 0, 0));
//
//    {
//        btScalar mass(0.);
//        DaerimsEngineBase::GetInstance().CreateRigidBody(mass, groundTransform, groundShape, 0.0f, btVector4(0, 0, 1, 1));
//    }
//
//    for (unsigned int i = 0; i < 1; i++)
//    {
//        btTransform t = btTransform(btQuaternion(0.0, 0.0, 0.0), btVector3(0, 0, stackZ -= 15.0f));
//        CreateStack(t, 8, 20, 2.5f);
//    }
//}
//void DaerimGame::UpdateLights(float dt) { AppBase::UpdateLights(dt); }
//
//void DaerimGame::Update(float dt) {
//
//    AppBase::Update(dt);
//
//    MeshLoadHelper::LoadAllUnloadedModel(m_device, m_context);
//    //m_wizardActor->Update(m_device,m_context,dt);
//
//    // 이하 물리엔진 관련
//    StepSimulation(dt);
//
//    btDiscreteDynamicsWorld* DynamicWorld = DaerimsEngineBase::GetInstance().GetDynamicWorld();
//    int count = 0;
//    int numCollisionObjects = DynamicWorld->getNumCollisionObjects();
//    {
//        for (int i = 0; i < numCollisionObjects; i++)
//        {
//            btCollisionObject* colObj = DynamicWorld->getCollisionObjectArray()[i];
//            if (colObj == nullptr)
//            {
//                continue;
//            }
//
//            if (!colObj->isStaticObject())
//            {
//                std::weak_ptr<Object> obj = DaerimsEngineBase::GetInstance().GetPhysObject(colObj);
//                if (obj.expired())
//                {
//                    DaerimsEngineBase::GetInstance().RemovePhysMap(colObj);
//                    continue;
//                }
//                std::shared_ptr<Object> objectLock = obj.lock();
//                if (objectLock->IsUsePhsycsSimulation() == false)
//                {
//                    continue;
//                }
//                // TODO count 인덱스로 찾는게 아닌 map으로 검색하도록 수정
//                btCollisionShape* collisionShape = colObj->getCollisionShape();
//                btVector3 pos = colObj->getWorldTransform().getOrigin();
//                btQuaternion orn = colObj->getWorldTransform().getRotation();
//                //Matrix::CreateFromQuaternion(orn.get128())*
//                //    Matrix::CreateTranslation(pos.get128())*
//                objectLock->UpdateWorldRow(Matrix::CreateTranslation(pos.get128()) *
//                    Matrix::CreateScale(m_simToRenderScale)); // PhysX to Render 스케일
//                //m_objectList[count]->UpdateConstantBuffers(m_device, m_context);
//                count++;
//            }
//        }
//    }
//}
//
////btRigidBody* DaerimGame::CreateDynamic(const btTransform& t,
////    btCollisionShape* shape,
////    const btVector3& velocity) {
////    m_fireball = std::make_shared<BillboardModel>();
////    //m_fireball->Initialize(m_device, m_context, {{0.0f, 0.0f, 0.0f, 1.0f}},
////    //                       1.0f, L"GameExplosionPS.hlsl");
////    Vector3 dir(float(velocity.getX()), float(velocity.getY()), float(velocity.getZ()));
////    dir.Normalize();
////    m_fireball->m_billboardConsts.m_cpu.directionWorld = dir;
////    m_fireball->m_castShadow = false;
////    m_fireball->Initialize(m_device, m_context, { {0.0f, 0.0f, 0.0f, 1.0f} },
////        0.2f, Graphics::volumetricFirePS);
////
////    AppBase::m_basicList.push_back(m_fireball);
////    m_objects.push_back(m_fireball);
////
////    btRigidBody* dynamic = 
////        DaerimsEngineBase::CreateRigidBody(m_dynamicsWorld,5.0, t, shape, 0.5f, btVector4(0, 0, 1, 1));
////    dynamic->setLinearVelocity(velocity);
////
////    return dynamic;
////}
//
//void DaerimGame::Render()
//{
//    AppBase::Render();
//    AppBase::PostRender();
//}
//
//void DaerimGame::UpdateGUI() {
//    AppBase::UpdateGUI();
//    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
//    if (ImGui::TreeNode("General")) {
//        ImGui::Checkbox("Use FPV", &m_camera.m_useFirstPersonView);
//        ImGui::Checkbox("Wireframe", &m_drawAsWire);
//        ImGui::Checkbox("DrawOBB", &m_drawOBB);
//        ImGui::Checkbox("DrawBSphere", &m_drawBS);
//        if (ImGui::Checkbox("MSAA ON", &m_useMSAA)) {
//            CreateBuffers();
//        }
//        ImGui::TreePop();
//    }
//
//    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//    if (ImGui::TreeNode("Skybox")) {
//        ImGui::SliderFloat("Strength", &m_globalConstsCPU.strengthIBL, 0.0f,
//            0.5f);
//        ImGui::RadioButton("Env", &m_globalConstsCPU.textureToDraw, 0);
//        ImGui::SameLine();
//        ImGui::RadioButton("Specular", &m_globalConstsCPU.textureToDraw, 1);
//        ImGui::SameLine();
//        ImGui::RadioButton("Irradiance", &m_globalConstsCPU.textureToDraw, 2);
//        ImGui::SliderFloat("EnvLodBias", &m_globalConstsCPU.envLodBias, 0.0f,
//            10.0f);
//        ImGui::TreePop();
//    }
//
//    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//    if (ImGui::TreeNode("Post Effects")) {
//        int flag = 0;
//        flag += ImGui::RadioButton("Render", &m_postEffectsConstsCPU.mode, 1);
//        ImGui::SameLine();
//        flag += ImGui::RadioButton("Depth", &m_postEffectsConstsCPU.mode, 2);
//        flag += ImGui::SliderFloat(
//            "DepthScale", &m_postEffectsConstsCPU.depthScale, 0.0, 1.0);
//        flag += ImGui::SliderFloat("Fog", &m_postEffectsConstsCPU.fogStrength,
//            0.0, 10.0);
//
//        if (flag)
//            D3D11Utils::UpdateBuffer(m_context, m_postEffectsConstsCPU,
//                m_postEffectsConstsGPU);
//
//        ImGui::TreePop();
//    }
//
//    if (ImGui::TreeNode("Post Processing")) {
//        int flag = 0;
//        flag += ImGui::SliderFloat(
//            "Bloom Strength",
//            &m_postProcess.m_combineFilter.m_constData.strength, 0.0f, 1.0f);
//        flag += ImGui::SliderFloat(
//            "Exposure", &m_postProcess.m_combineFilter.m_constData.option1,
//            0.0f, 10.0f);
//        flag += ImGui::SliderFloat(
//            "Gamma", &m_postProcess.m_combineFilter.m_constData.option2, 0.1f,
//            5.0f);
//        // 편의상 사용자 입력이 인식되면 바로 GPU 버퍼를 업데이트
//        if (flag) {
//            m_postProcess.m_combineFilter.UpdateConstantBuffers(m_context);
//        }
//        ImGui::TreePop();
//    }
//
//    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//    if (m_mirror && ImGui::TreeNode("Mirror")) {
//
//        ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
//        const float blendColor[4] = { m_mirrorAlpha, m_mirrorAlpha,
//                                     m_mirrorAlpha, 1.0f };
//        if (m_drawAsWire)
//            Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
//        else
//            Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);
//
//        ImGui::SliderFloat("Metallic",
//            &m_mirror->m_materialConsts.GetCpu().metallicFactor,
//            0.0f, 1.0f);
//        ImGui::SliderFloat("Roughness",
//            &m_mirror->m_materialConsts.GetCpu().roughnessFactor,
//            0.0f, 1.0f);
//
//        ImGui::TreePop();
//    }
//
//    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//    if (ImGui::TreeNode("Light")) {
//        // ImGui::SliderFloat3("Position",
//        // &m_globalConstsCPU.lights[0].position.x,
//        //                     -5.0f, 5.0f);
//        ImGui::SliderFloat("Halo Radius",
//            &m_globalConstsCPU.lights[1].haloRadius, 0.0f, 2.0f);
//        ImGui::SliderFloat("Halo Strength",
//            &m_globalConstsCPU.lights[1].haloStrength, 0.0f,
//            1.0f);
//        ImGui::SliderFloat("Radius", &m_globalConstsCPU.lights[1].radius, 0.0f,
//            0.5f);
//        ImGui::TreePop();
//    }
//
//    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//    if (ImGui::TreeNode("Material")) {
//        ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias, 0.0f, 10.0f);
//
//        int flag = 0;
//
//        if (m_pickedModel) {
//            flag += ImGui::SliderFloat(
//                "Metallic",
//                &m_pickedModel->m_materialConsts.GetCpu().metallicFactor, 0.0f,
//                1.0f);
//            flag += ImGui::SliderFloat(
//                "Roughness",
//                &m_pickedModel->m_materialConsts.GetCpu().roughnessFactor, 0.0f,
//                1.0f);
//            flag += ImGui::CheckboxFlags(
//                "AlbedoTexture",
//                &m_pickedModel->m_materialConsts.GetCpu().useAlbedoMap, 1);
//            flag += ImGui::CheckboxFlags(
//                "EmissiveTexture",
//                &m_pickedModel->m_materialConsts.GetCpu().useEmissiveMap, 1);
//            flag += ImGui::CheckboxFlags(
//                "Use NormalMapping",
//                &m_pickedModel->m_materialConsts.GetCpu().useNormalMap, 1);
//            flag += ImGui::CheckboxFlags(
//                "Use AO", &m_pickedModel->m_materialConsts.GetCpu().useAOMap,
//                1);
//            flag += ImGui::CheckboxFlags(
//                "Use HeightMapping",
//                &m_pickedModel->m_meshConsts.GetCpu().useHeightMap, 1);
//            flag += ImGui::SliderFloat(
//                "HeightScale",
//                &m_pickedModel->m_meshConsts.GetCpu().heightScale, 0.0f, 0.1f);
//            flag += ImGui::CheckboxFlags(
//                "Use MetallicMap",
//                &m_pickedModel->m_materialConsts.GetCpu().useMetallicMap, 1);
//            flag += ImGui::CheckboxFlags(
//                "Use RoughnessMap",
//                &m_pickedModel->m_materialConsts.GetCpu().useRoughnessMap, 1);
//            if (flag) {
//                m_pickedModel->UpdateConstantBuffers(m_device, m_context);
//            }
//            ImGui::Checkbox("Draw Normals", &m_pickedModel->m_drawNormals);
//        }
//
//        ImGui::TreePop();
//    }
//}
//
//void DaerimGame::CreateStack(const btTransform t, int numStacks,
//    int numWidth, btScalar halfExtent) {
//    //vector<MeshData> box = { GeometryGenerator::MakeBox(halfExtent) };
//    btTransform groundTransform;
//    groundTransform.setIdentity();
//    groundTransform.setOrigin(btVector3(0, -50, 0));
//
//    //	//create a few dynamic rigidbodies
////	// Re-using the same collision is better for memory usage and performance
//
//    btBoxShape* colShape = DaerimsEngineBase::GetInstance().CreateBoxShape(btVector3(halfExtent, halfExtent, halfExtent));
//
//    //btCollisionShape* colShape = new btSphereShape(btScalar(1.));
//    m_collisionShapes.push_back(colShape);
//
//    /// Create Dynamic Objects
//    btTransform startTransform;
//    startTransform.setIdentity();
//
//    btScalar mass(1.f);
//
//    //rigidbody is dynamic if and only if mass is non zero, otherwise static
//    bool isDynamic = (mass != 0.f);
//
//    btVector3 localInertia(0, 0, 0);
//    if (isDynamic)
//        colShape->calculateLocalInertia(mass, localInertia);
//    string meshKey = MeshLoadHelper::LoadBoxMesh(m_device, m_context, halfExtent);
//    for (int i = 0; i < numStacks; i++)
//    {
//        for (int j = 0; j < numWidth - i; j++)
//        {
//            btTransform localTm;
//            localTm.setIdentity();
//            localTm.setOrigin(btVector3(btScalar(j * 2) - btScalar(numWidth - i),
//                btScalar(i * 2 + 1) + 5.0, 0.0) *
//                halfExtent);
//            localTm.setBasis(btMatrix3x3::getIdentity());
//            btRigidBody* body = DaerimsEngineBase::GetInstance().CreateRigidBody(mass, t * localTm, colShape, 0.0f, btVector4(0, 0, 1, 1));
//
//            auto newModel = std::make_shared<DModel>(
//                m_device, m_context, meshKey); // <- 우리 렌더러에 추가
//            newModel->m_materialConsts.GetCpu().albedoFactor = Vector3(0.8f);
//
//            auto newObj = std::make_shared<Object>();
//            newObj->Initialize(m_device, m_context, newModel);
//            newObj->SetPhysicsBody(body);
//
//            DaerimsEngineBase::GetInstance().RegisterPhysMap(body, newObj);
//            m_objectList.push_back(newObj);
//            newObj->SetUsePhsycisSimulation(true);
//        }
//    }
//}
//
//void DaerimGame::StepSimulation(float deltaTime)
//{
//    DaerimsEngineBase::GetInstance().StepSimulation(deltaTime);
//}
//}