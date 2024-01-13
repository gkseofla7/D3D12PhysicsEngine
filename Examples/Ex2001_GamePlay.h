#pragma once

#include "AppBase.h"
#include "BillboardModel.h"
#include "PxPhysicsAPI.h"
#include "SkinnedMeshModel.h"

// 기본 사용법은 SnippetHelloWorld.cpp
// 렌더링 관련은 SnippetHelloWorldRender.cpp

#define PX_RELEASE(x)                                                          \
    if (x) {                                                                   \
        x->release();                                                          \
        x = NULL;                                                              \
    }

#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 128

namespace hlab {

using namespace physx;

class Ex2001_GamePlay : public AppBase {
  public:
    Ex2001_GamePlay();

    ~Ex2001_GamePlay() {
        PX_RELEASE(gScene);
        PX_RELEASE(gDispatcher);
        PX_RELEASE(gPhysics);
        if (gPvd) {
            PxPvdTransport *transport = gPvd->getTransport();
            gPvd->release();
            gPvd = NULL;
            PX_RELEASE(transport);
        }
        PX_RELEASE(gFoundation);
    }

    bool InitScene() override;

    PxRigidDynamic *CreateDynamic(const PxTransform &t,
                                  const PxGeometry &geometry,
                                  const PxVec3 &velocity = PxVec3(0));
    void CreateStack(const PxTransform &t, int numStacks, int numSlices,
                     PxReal halfExtent);
    void InitPhysics(bool interactive);
    void UpdateLights(float dt) override;
    void UpdateGUI() override;
    void Update(float dt) override;
    void Render() override;

  public:
    float m_simToRenderScale = 0.01f; // 시뮬레이션 물체가 너무 작으면 불안정

    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;
    PxFoundation *gFoundation = NULL;
    PxPhysics *gPhysics = NULL;
    PxDefaultCpuDispatcher *gDispatcher = NULL;
    PxScene *gScene = NULL;
    PxMaterial *gMaterial = NULL;
    PxPvd *gPvd = NULL;
    PxReal stackZ = 10.0f;

    vector<shared_ptr<Model>>
        m_objects; // 물리 엔진과 동기화 시켜줄 때 사용 TODO: actor list로 변경

    shared_ptr<BillboardModel> m_fireball;
    shared_ptr<SkinnedMeshModel> m_character;
};

} // namespace hlab
