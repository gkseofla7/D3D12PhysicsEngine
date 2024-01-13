#pragma once

#include "Model.h"

namespace hlab {

class OceanModel : public Model {
  public:
    OceanModel(ComPtr<ID3D11Device> &device,
               ComPtr<ID3D11DeviceContext> &context,
               const vector<MeshData> &meshes)
        : Model(device, context, meshes) {}

    GraphicsPSO &GetPSO(const bool wired) override {
        return wired ? Graphics::defaultWirePSO : Graphics::oceanPSO;
    }
};

} // namespace hlab