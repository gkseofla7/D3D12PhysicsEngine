#pragma once

#include "AppBase.h"
#include "Model.h"
#include "StableFluids.h"

namespace hlab {

class Ex1601_StableFluids : public AppBase {
  public:
    Ex1601_StableFluids();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    StableFluids m_stableFluids;
};

} // namespace hlab