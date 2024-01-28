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
        AnimNum,
    };

class SkeletalMeshActor : public Actor {
public:
    // 입력 관련 정보는 따로 관리하도록 하자, State랑 Animation 분리가 필요하다고 본다.
    void UpdateAnimation(ComPtr<ID3D11DeviceContext> m_context, float dt, bool* keyPressed);
    void InitAnimationData(ComPtr<ID3D11Device>& device,
        const AnimationData& aniData);
    //void UpdateState();
private:
    int state = 0;
public:
    shared_ptr< class SkinnedMeshModel> m_skinnedMeshModel;
    StructuredBuffer<Matrix> m_boneTransforms;
};
} // namespace hlab