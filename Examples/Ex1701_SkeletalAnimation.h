#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>

#include "AnimationClip.h"
#include "AppBase.h"
#include "GeometryGenerator.h"
#include "ImageFilter.h"
#include "Model.h"
#include "SkinnedMeshModel.h"

//enum WalkingAnimState {
//  Idle = 0,
//  IdleToWalk = 1,
//  Walking = 2,
//  WalkingBackward = 3,
//  WalkingToIdle = 4,
//  CombatStateStart = 5,
//  Punching = 6,
//  Hook = 7,
//  CombatState = 8,
//  AnimNum,
//};

enum WalkingAnimState {
    Idle = 0,
    IdleToWalk = 1,
    Walking = 2,
    WalkingBackward = 3,
    WalkingToIdle= 4,
    CombatStateStart = 5,
    CombatState = 6,
    AnimNum,
};

enum SpecialAnimState {
  Punching = 7,
  Hook = 8,
  None = 9,
};


namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

class Ex1701_SkeletalAnimation : public AppBase {
  public:
    Ex1701_SkeletalAnimation();

    virtual bool InitScene() override;

    void UpdateLights(float dt) override;
    void UpdateGUI() override;
    void Update(float dt) override;
    void Render() override;
    
  protected:
    shared_ptr<Model> m_ground;

    shared_ptr<SkinnedMeshModel> m_character; 
    WalkingAnimState m_walkingState = WalkingAnimState::Idle;
    SpecialAnimState m_specialState = SpecialAnimState::None;
    map<int, string> AnimStateToAnim;
    bool m_playingSpecialAT = false;
};

} // namespace hlab
