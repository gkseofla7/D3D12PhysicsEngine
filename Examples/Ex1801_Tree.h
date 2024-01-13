#pragma once

#include "AppBase.h"
#include "Model.h"

namespace hlab {

class Ex1801_Tree : public AppBase {
  public:
    Ex1801_Tree();

    virtual bool InitScene() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

  protected:
    shared_ptr<Model> m_ground;
    shared_ptr<Model> m_leaves;
    shared_ptr<Model> m_trunk;
};

} // namespace hlab