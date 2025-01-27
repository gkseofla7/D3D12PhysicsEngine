#pragma once

#include "Legacy/AppBase.h"
#include "Legacy/Model.h"

namespace hlab {

class Example_TEMPLATE : public AppBase {
  public:
    Example_TEMPLATE();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
};

} // namespace hlab