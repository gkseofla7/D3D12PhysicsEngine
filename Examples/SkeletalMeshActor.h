#pragma once

#include "Actor.h"

namespace hlab {
    enum CommonAnimState {
        Idle = 0,
        IdleToWalk = 1,
        Walking = 2,
        WalkingBackward = 3,
        WalkingToIdle = 4,
        Specail = 5,
    };

class SkeletalMeshActor : public Actor {
public:
    // 입력 관련 정보는 따로 관리하도록 하자, State랑 Animation 분리가 필요하다고 본다.
    //void UpdateAnimation(ComPtr<ID3D11DeviceContext> m_context, float dt, bool* keyPressed);
    //void InitAnimationData(ComPtr<ID3D11Device>& device,
    //    const AnimationData& aniData);
    //void UpdateState();

public:
    SkeletalMeshActor() {}
    SkeletalMeshActor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  InModel);
    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  InModel);
    virtual void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt) override;
    virtual void InitializeAnimation() {}
    //TODO. UpdateAnimation에서 쓸때 비동기 로딩을 하기 때문에(defered..) 
    // 초기화 마저 UpdateAnimation내부에서 하게된다.
    // device, context를 안받게 만들고싶은데..ㅋㅋ 방법을 고민해보자
    void UpdateAnimation(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, int clipId,
        int frame, int type = 0);

    void Render(ComPtr<ID3D11DeviceContext>& context) override;

    // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);
    void UpdateVelocity(float dt) {
        //Vector3 prevPos = m_prevRootTransform.Translation();
        //Vector3 curPos = m_aniData.accumulatedRootTransform.Translation();

        //m_velocity = (curPos - prevPos).Length() / dt;
        //m_prevRootTransform = m_aniData.accumulatedRootTransform;
    }
public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;

    Matrix accumulatedRootTransform = Matrix();
    float m_velocity = 0.0f;
    Matrix m_prevRootTransform;

    //Anim 관련
    int m_curFrame;
private:
    int m_state = 0;
};
} // namespace hlab