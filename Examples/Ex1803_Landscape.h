#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>

#include "AppBase.h"
#include "GeometryGenerator.h"
#include "ImageFilter.h"
#include "Model.h"

namespace hlab {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

class Ex1803_Landscape : public AppBase {
  public:
    Ex1803_Landscape();

    bool InitScene() override;

    void UpdateLights(float dt) override;
    void UpdateGUI() override;
    void Update(float dt) override;
    void Render() override;

  protected:
    shared_ptr<Model> m_ocean;
    shared_ptr<Model> m_terrain;
};

} // namespace hlab
