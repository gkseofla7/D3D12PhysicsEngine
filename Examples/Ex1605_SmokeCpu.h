#pragma once

#include "AppBase.h"
#include "FluidSimulationCPU.h"

namespace hlab {

class Ex1605_SmokeCpu : public AppBase {
  public:
    Ex1605_SmokeCpu();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    shared_ptr<Model> m_volumeModel;
    vector<float> m_volumeData;

    ConstantBuffer<VolumeConsts> m_volumeConsts;

    FluidSimulationCPU m_fluid;
};

} // namespace hlab