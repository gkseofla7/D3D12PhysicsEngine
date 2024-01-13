#include "Ex1408_BitonicSort.h"
#include "BitonicSort.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1408_BitonicSort::Ex1408_BitonicSort() : AppBase() {}

bool Ex1408_BitonicSort::Initialize() {

    cout << "Ex1408_BitonicSort::Initialize()" << endl;

    if (!AppBase::Initialize())
        return false;

    BitonicSort::TestBitonicSort(m_device, m_context);

    return true;
}

void Ex1408_BitonicSort::Update(float dt) {}

void Ex1408_BitonicSort::Render() {}

void Ex1408_BitonicSort::UpdateGUI() {}

} // namespace hlab