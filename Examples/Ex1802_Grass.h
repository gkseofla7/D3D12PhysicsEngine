#pragma once

#include "AppBase.h"
#include "GrassModel.h"

namespace hlab {

class Ex1802_Grass : public AppBase {
  public:
    Ex1802_Grass();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    shared_ptr<Model> m_ground;
    shared_ptr<GrassModel> m_grass;
    float m_windStrength = 1.0f;
};

} // namespace hlab