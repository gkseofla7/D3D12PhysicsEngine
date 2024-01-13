#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1408_BitonicSort : public AppBase {
  public:
    Ex1408_BitonicSort();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
};

} // namespace hlab