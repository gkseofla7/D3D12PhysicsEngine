#include "Ex1601_StableFluids.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1601_StableFluids::Ex1601_StableFluids() : AppBase() {}

bool Ex1601_StableFluids::Initialize() {

    cout << "Ex1601_StableFluids::Initialize()" << endl;

    m_screenWidth = 1024;
    m_screenHeight = 1024;

    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    if (!AppBase::Initialize())
        return false;

    m_stableFluids.Initialize(m_device, m_screenWidth, m_screenHeight);

    return true;
}

void Ex1601_StableFluids::Update(float dt) {

    static int color = 0;
    static bool m_prevLeftButton = false;
    static float m_prevMouseNdcX = -1.0f;
    static float m_prevMouseNdcY = -1.0f;

    if (AppBase::m_leftButton) {

        m_stableFluids.m_constsCPU.i = AppBase::m_mouseX;
        m_stableFluids.m_constsCPU.j = AppBase::m_mouseY;

        if (!m_prevLeftButton) { // 버튼을 새로 누른 경우

            // 랜덤하게 색 교체
            static const std::vector<Vector4> rainbow = {
                {1.0f, 0.0f, 0.0f, 1.0f},  // Red
                {1.0f, 0.65f, 0.0f, 1.0f}, // Orange
                {1.0f, 1.0f, 0.0f, 1.0f},  // Yellow
                {0.0f, 1.0f, 0.0f, 1.0f},  // Green
                {0.0f, 0.0f, 1.0f, 1.0f},  // Blue
                {0.3f, 0.0f, 0.5f, 1.0f},  // Indigo
                {0.5f, 0.0f, 1.0f, 1.0f}   // Violet/Purple
            };

            m_stableFluids.m_constsCPU.sourcingDensity = rainbow[(color++) % 7];
            m_stableFluids.m_constsCPU.sourcingVelocity = Vector2(0.0f);

        } else {
            Vector2 ndcVel = Vector2(m_mouseNdcX, -m_mouseNdcY) -
                             Vector2(m_prevMouseNdcX, -m_prevMouseNdcY);
            m_stableFluids.m_constsCPU.sourcingVelocity = ndcVel * 10.0f;
        }
    } else {
        m_stableFluids.m_constsCPU.i = -1; // uint의 Overflow 이용
    }

    m_prevLeftButton = AppBase::m_leftButton;
    m_prevMouseNdcX = AppBase::m_mouseNdcX;
    m_prevMouseNdcY = AppBase::m_mouseNdcY;

    m_stableFluids.Update(m_device, m_context, dt);
}

void Ex1601_StableFluids::Render() {

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    m_context->CopyResource(backBuffer.Get(),
                            m_stableFluids.m_density.GetTexture());
}

void Ex1601_StableFluids::UpdateGUI() {}

} // namespace hlab