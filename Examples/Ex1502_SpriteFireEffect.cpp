#include "Ex1502_SpriteFireEffect.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1502_SpriteFireEffect::Ex1502_SpriteFireEffect() : AppBase() {}

bool Ex1502_SpriteFireEffect::Initialize() {

    cout << "Ex1502_SpriteFireEffect::Initialize()" << endl;

    // ComputeShader에서 Backbuffer를 사용
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    m_screenWidth = 1280;
    m_screenHeight = 1280;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화
    m_particlesCPU.resize(1024);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dp(-1.0f, 1.0f);
    for (auto &p : m_particlesCPU) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = Vector3(1.0f, 0.8f, 0.0f);
        // p.size = 1.0f; // 파티클 sourcing 시에 설정
        p.life = -1.0f;
    }

    D3D11Utils::CreateStructuredBuffer(
        m_device, UINT(m_particlesCPU.size()), sizeof(Particle),
        m_particlesCPU.data(), m_particlesGPU, m_particlesSRV, m_particlesUAV);

    D3D11Utils::CreateStagingBuffer(m_device, UINT(m_particlesCPU.size()),
                                    sizeof(Particle), m_particlesCPU.data(),
                                    m_particlesStagingGPU);

    // 주의: Vertex Shader에서 Vertex 정보 미사용

    // GS는 ParticleSystem 예제와 동일
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1502_SpriteFireEffectVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1502_SpriteFireEffectPS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateGeometryShader(m_device, L"Ex1501_ParticleSystemGS.hlsl",
                                     m_spriteGS);

    // Sprite Texture
    D3D11Utils::CreateDDSTexture(m_device, L"../Assets/Textures/flare0.dds",
                                 false, m_spriteSRV);

    return true;
}

void Ex1502_SpriteFireEffect::Update(float dt) {

    dt *= 1.0f; // 느리게 진행하고 싶을 경우 가상의 시간 사용 가능

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> randomTheta(-3.141592f, 3.141592f);
    uniform_real_distribution<float> randomSpeed(1.5f, 2.0f);
    uniform_real_distribution<float> randomLife(0.0f, 1.0f);

    int newCount = 20; // 한 프레임에 새로 만들어질 수 있는 파티클 개수
    for (auto &p : m_particlesCPU) {
        if (p.life < 0.0f && newCount > 0) {

            // TODO:

            newCount--;
        }
    }

    // 마우스 클릭시 추가
    newCount = 20;
    for (auto &p : m_particlesCPU) {
        if (AppBase::m_leftButton && p.life < 0.0f && newCount > 0) {

            // TODO:

            newCount--;
        }
    }

    // const Vector3 gravity = Vector3(0.0f, -9.8f, 0.0f);
    const Vector3 buoyancy = Vector3(0.0f, 2.0f, 0.0f);

    int countActive = 0;
    for (auto &p : m_particlesCPU) {

        if (p.life < 0.0f)
            continue;

        // TODO:

        countActive++;
    }

    // cout << countActive << endl;

    D3D11Utils::CopyToStagingBuffer(
        m_context, m_particlesStagingGPU,
        UINT(sizeof(Particle) * m_particlesCPU.size()), m_particlesCPU.data());
    m_context->CopyResource(m_particlesGPU.Get(), m_particlesStagingGPU.Get());
}

void Ex1502_SpriteFireEffect::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    DrawSprites();

    // timer.End(m_context);
}

void Ex1502_SpriteFireEffect::DrawSprites() {

    // Geometry Shader로 Particle Sprites 그리기

    AppBase::SetMainViewport();

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), NULL);

    m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
    m_context->GSSetShader(m_spriteGS.Get(), 0, 0);
    m_context->PSSetShader(m_pixelShader.Get(), 0, 0);
    m_context->CSSetShader(NULL, 0, 0);

    // 색을 모두 더하면서 그리는 accumulateBS 사용
    const float blendColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_context->OMSetBlendState(Graphics::accumulateBS.Get(), blendColor,
                               0xffffffff);

    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    m_context->VSSetShaderResources(0, 1, m_particlesSRV.GetAddressOf());

    // TODO:

    m_context->PSSetSamplers(0, 1, Graphics::linearWrapSS.GetAddressOf());
    m_context->Draw(UINT(m_particlesCPU.size()), 0);
}

void Ex1502_SpriteFireEffect::UpdateGUI() {}

} // namespace hlab