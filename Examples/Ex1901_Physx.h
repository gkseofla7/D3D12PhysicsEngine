#pragma once

#include "AppBase.h"

#include "PxPhysicsAPI.h"

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

class Ex1901_PhysX : public AppBase {
  public:
    Ex1901_PhysX();

    ~Ex1901_PhysX() {
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

    PxRigidDynamic *createDynamic(const PxTransform &t,
                                  const PxGeometry &geometry,
                                  const PxVec3 &velocity = PxVec3(0)) {
        PxRigidDynamic *dynamic =
            PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
        dynamic->setAngularDamping(0.5f);
        dynamic->setLinearVelocity(velocity);
        gScene->addActor(*dynamic);
        return dynamic;
    }

    void createStack(const PxTransform &t, PxU32 size, PxReal halfExtent) {

        vector<MeshData> box = {GeometryGenerator::MakeBox(halfExtent)};

        PxShape *shape = gPhysics->createShape(
            PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);

        for (PxU32 i = 2; i < size; i++) {
            for (PxU32 j = 0; j < size - i; j++) {
                PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i),
                                           PxReal(i * 2 + 1), 0) *
                                    halfExtent);
                PxRigidDynamic *body =
                    gPhysics->createRigidDynamic(t.transform(localTm));
                body->attachShape(*shape);
                PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
                gScene->addActor(*body);

                auto m_newObj =
                    std::make_shared<Model>(m_device, m_context, box);
                m_newObj->m_materialConsts.GetCpu().albedoFactor =
                    Vector3(0.5f);
                AppBase::m_basicList.push_back(m_newObj);
                this->m_objects.push_back(m_newObj);
            }
        }
        shape->release();
    }

    void InitPhysics(bool interactive);

    bool InitScene() override;

    void UpdateLights(float dt) override;
    void UpdateGUI() override;
    void Update(float dt) override;
    void Render() override;

  public:
    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;
    PxFoundation *gFoundation = NULL;
    PxPhysics *gPhysics = NULL;
    PxDefaultCpuDispatcher *gDispatcher = NULL;
    PxScene *gScene = NULL;
    PxMaterial *gMaterial = NULL;
    PxPvd *gPvd = NULL;
    PxReal stackZ = 10.0f;

    vector<shared_ptr<Model>> m_objects;
};

} // namespace hlab
