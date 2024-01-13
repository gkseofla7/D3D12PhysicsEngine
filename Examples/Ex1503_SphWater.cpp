#include "Ex1503_SphWater.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1503_SphWater::Ex1503_SphWater() : AppBase() {}

bool Ex1503_SphWater::Initialize() {

    cout << "Ex1503_SphWater::Initialize()" << endl;

    // ComputeShader에서 Backbuffer를 사용
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    m_screenWidth = 1024;
    m_screenHeight = 1024;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화 (최대 Particle 수가 정해져있는 구조)
    m_sph.m_particlesCpu.resize(1024 * 2);

    vector<Vector3> rainbow = {
        {1.0f, 0.0f, 0.0f},  // Red
        {1.0f, 0.65f, 0.0f}, // Orange
        {1.0f, 1.0f, 0.0f},  // Yellow
        {0.0f, 1.0f, 0.0f},  // Green
        {0.0f, 0.0f, 1.0f},  // Blue
        {0.3f, 0.0f, 0.5f},  // Indigo
        {0.5f, 0.0f, 1.0f}   // Violet/Purple
    };

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dp(-1.0f, 1.0f);
    uniform_int_distribution<size_t> dc(0, rainbow.size() - 1);
    for (auto &p : m_sph.m_particlesCpu) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = rainbow[dc(gen)];
        p.size = m_sph.m_radius;
        p.life = -1.0f;
    }

    D3D11Utils::CreateStructuredBuffer(
        m_device, UINT(m_sph.m_particlesCpu.size()),
        sizeof(SphSimulation::Particle), m_sph.m_particlesCpu.data(),
        m_particlesGPU, m_particlesSRV, m_particlesUAV);

    D3D11Utils::CreateStagingBuffer(m_device, UINT(m_sph.m_particlesCpu.size()),
                                    sizeof(SphSimulation::Particle),
                                    m_sph.m_particlesCpu.data(),
                                    m_particlesStagingGPU);

    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1503_SphWaterVS.hlsl", inputElements, m_vertexShader,
        m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1503_SphWaterPS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateGeometryShader(m_device, L"Ex1501_ParticleSystemGS.hlsl",
                                     m_spriteGS);

    return true;
}

void Ex1503_SphWater::Update(float dt) {

    dt = 1.0f / 60.0f * 0.25f; // 고정

    int newCount = 50; // 한 프레임에 새로 만들어질 수 있는 파티클 개수
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> randomTheta(-3.141592f, 3.141592f);
    uniform_real_distribution<float> randomSpeed(1.5f, 2.0f);
    uniform_real_distribution<float> randomLife(0.0f, 1.0f);

    // 마우스 클릭시 추가 (이 예제에서는 미사용)
    /* for (auto &p : m_sph.m_particlesCpu) {

        // 비활성화되어 있는 입자를 찾으면 활성화하는 방식
        if (AppBase::m_leftButton && p.life < 0.0f && newCount > 0) {

            const float theta = randomTheta(gen);

            p.position = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0);
            p.velocity =
                Vector3(cos(theta), -sin(theta), 0.0) * randomSpeed(gen);
            p.life = randomLife(gen) * 1.5f;
            newCount--;
        }
    }*/

    // 항상 추가하는 Source
    newCount = 1;
    for (auto &p : m_sph.m_particlesCpu) {
        if (p.life < 0.0f && newCount > 0) {

            const float theta = randomTheta(gen);

            p.position =
                Vector3(cos(theta), -sin(theta), 0.0) * randomLife(gen) * 0.1f +
                Vector3(-0.5f, 0.5f, 0.0f);
            p.velocity = Vector3(1.0f, 0.0f, 0.0);
            p.life = randomLife(gen) * 1.5f;
            p.size = m_sph.m_radius;
            newCount--;
        }
    }

    newCount = 1;
    for (auto &p : m_sph.m_particlesCpu) {
        if (p.life < 0.0f && newCount > 0) {

            const float theta = randomTheta(gen);

            p.position =
                Vector3(cos(theta), -sin(theta), 0.0) * randomLife(gen) * 0.1f +
                Vector3(0.5f, 0.5f, 0.0f);
            p.velocity = Vector3(-1.0f, 0.0f, 0.0);
            p.life = randomLife(gen) * 1.5f;
            p.size = m_sph.m_radius;
            newCount--;
        }
    }

    m_sph.Update(dt);

    const Vector3 gravity = Vector3(0.0f, -9.8f, 0.0f);
    const float cor = 0.5f; // Coefficient Of Restitution
    const float groundHeight = -0.8f;

    for (auto &p : m_sph.m_particlesCpu) {

        if (p.life < 0.0f)
            continue;

        p.velocity += gravity * dt;
        // p.position += p.velocity * dt;
        // p.life -= dt;

        if (p.position.y < groundHeight && p.velocity.y < 0.0f) {
            p.velocity.y *= -cor;
            // p.velocity.y *= -randomLife(gen);
            p.position.y = groundHeight;
            // p.life = -1.0; // 제거
        }

        if (p.position.x < -0.9f && p.velocity.x < 0.0f) {
            p.velocity.x *= -cor;
            // p.velocity.x *= -randomLife(gen);
            p.position.x = -0.9f;
            // p.life = -1.0; // 제거
        }

        if (p.position.x > 0.9f && p.velocity.x > 0.0f) {
            p.velocity.x *= -cor;
            p.position.x = 0.9f;
            // p.life = -1.0; // 제거
        }
    }

    D3D11Utils::CopyToStagingBuffer(
        m_context, m_particlesStagingGPU,
        UINT(sizeof(SphSimulation::Particle) * m_sph.m_particlesCpu.size()),
        m_sph.m_particlesCpu.data());
    m_context->CopyResource(m_particlesGPU.Get(), m_particlesStagingGPU.Get());
}

void Ex1503_SphWater::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    DrawSprites();

    // timer.End(m_context);
}

void Ex1503_SphWater::DrawSprites() {

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
    m_context->Draw(UINT(m_sph.m_particlesCpu.size()), 0);
}

void Ex1503_SphWater::UpdateGUI() {}

} // namespace hlab