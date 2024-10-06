#pragma once

#include "Actor.h"

namespace hlab {
    class ActorState;
    class DSkinnedMeshModel;
class SkeletalMeshActor : public Actor {
public:
    SkeletalMeshActor() {}
    SkeletalMeshActor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  InModel);
    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  InModel);
    virtual void Tick(float dt) override;
    virtual void InitializeAnimation() {}

    void Render(ComPtr<ID3D11DeviceContext>& context) override;

    // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);
public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;
    //Anim 관련
    int m_curFrame = 0;
private:
    int m_state = 0;

};
} // namespace hlab