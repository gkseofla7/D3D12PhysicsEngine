#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "DModel2.h"

namespace dengine {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using std::vector;

struct BillboardConsts {
    float widthWorld;
    Vector3 directionWorld;
};

static_assert((sizeof(BillboardConsts) % 16) == 0,
              "Constant Buffer size must be 16-byte aligned");

class BillboardModel : public DModel {
  public:
    void Initialize(const std::vector<Vector4> &points, const float width);

    virtual void Render() override;
    virtual bool IsPostProcess() { return false; }
  private:
    ConstantBuffer<BillboardConsts> m_billboardConsts;

    uint32_t m_indexCount = 0;
};

} // namespace hlab