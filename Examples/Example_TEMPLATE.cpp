#include "Example_TEMPLATE.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Example_TEMPLATE::Example_TEMPLATE() : AppBase() {}

bool Example_TEMPLATE::InitScene() {

    cout << "Example_TEMPLATE::InitScene()" << endl;

    AppBase::m_camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);
    AppBase::InitCubemaps(L"../Assets/Textures/Cubemaps/HDRI/",
                          L"SampleEnvHDR.dds", L"SampleSpecularHDR.dds",
                          L"SampleDiffuseHDR.dds", L"SampleBrdf.dds");

    AppBase::InitScene();

    return true;
}

void Example_TEMPLATE::Update(float dt) { AppBase::Update(dt); }

void Example_TEMPLATE::Render() {
    AppBase::Render();
    AppBase::PostRender();
}

void Example_TEMPLATE::UpdateGUI() { AppBase::UpdateGUI(); }

} // namespace hlab