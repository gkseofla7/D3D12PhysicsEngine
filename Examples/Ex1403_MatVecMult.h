#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1403_MatVecMult : public AppBase {
  public:
    Ex1403_MatVecMult();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    void PrepareForData();
    void TestCPU();
    void TestGPU();

    vector<float> m_myMat;
    vector<float> m_myVec;
    vector<float> m_myResult;
    vector<float> m_myResultGPU;

    // 실험 크기는 Initialize()에서 수정 가능
    int m_repeat = 100;
    int m_numRows = 1024;
    int m_numCols = 2048;
};

} // namespace hlab