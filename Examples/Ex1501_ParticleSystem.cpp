#include "Ex1501_ParticleSystem.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"

#include <numeric>
#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1501_ParticleSystem::Ex1501_ParticleSystem() : AppBase() {}

bool Ex1501_ParticleSystem::Initialize() {

    cout << "Ex1501_ParticleSystem::Initialize()" << endl;

    // ComputeShader에서 Backbuffer를 사용
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    m_screenWidth = 1024;
    m_screenHeight = 1024;

    if (!AppBase::Initialize())
        return false;

    // 1. 데이터 초기화 (최대 Particle 수가 정해져있는 구조)
    m_particlesCPU.resize(2048);

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
    for (auto &p : m_particlesCPU) {
        p.position = Vector3(dp(gen), dp(gen), 1.0f);
        p.color = rainbow[dc(gen)];
        p.radius = (dp(gen) + 1.3f) * 0.02f;
        p.life = -1.0f;
    }

    D3D11Utils::CreateStructuredBuffer(
        m_device, UINT(m_particlesCPU.size()), sizeof(Particle),
        m_particlesCPU.data(), m_particlesGPU, m_particlesSRV, m_particlesUAV);

    D3D11Utils::CreateStagingBuffer(m_device, UINT(m_particlesCPU.size()),
                                    sizeof(Particle), m_particlesCPU.data(),
                                    m_particlesStagingGPU);

    // 주의: Vertex Shader에서 Vertex 정보 미사용

    // VS는 이전 예제와 동일
    const vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0}}; // Dummy
    D3D11Utils::CreateVertexShaderAndInputLayout(
        m_device, L"Ex1501_ParticleSystemVS.hlsl", inputElements,
        m_vertexShader, m_inputLayout);
    D3D11Utils::CreatePixelShader(m_device, L"Ex1501_ParticleSystemPS.hlsl",
                                  m_pixelShader);
    D3D11Utils::CreateGeometryShader(m_device, L"Ex1501_ParticleSystemGS.hlsl",
                                     m_spriteGS);

    return true;
}

void Ex1501_ParticleSystem::Update(float dt) {

    dt *= 0.5f; // 느리게 진행하고 싶을 경우 가상의 시간 사용 가능

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> randomTheta(-3.141592f, 3.141592f);
    uniform_real_distribution<float> randomSpeed(1.5f, 2.0f);
    uniform_real_distribution<float> randomLife(0.0f, 1.0f);

    // 마우스 클릭시 추가
    int newCount = 50; // 한 프레임에 새로 만들어질 수 있는 파티클 개수
    for (auto &p : m_particlesCPU) {
        // TODO:
    }

    // 항상 추가하는 Source
    newCount = 10;
    for (auto &p : m_particlesCPU) {

        // 비활성화되어 있는 입자를 찾으면 활성화하는 방식
        if (p.life < 0.0f && newCount > 0) {

            const float theta = randomTheta(gen);
            p.position =
                Vector3(cos(theta), -sin(theta), 0.0) * randomLife(gen) * 0.1f +
                Vector3(0.0f, -0.3f, 0.0f);
            p.velocity = Vector3(-1.0f, 0.0f, 0.0) * randomSpeed(gen);
            p.life = randomLife(gen) * 1.5f; // 수명 추가
            newCount--;
        }
    }

    const Vector3 gravity = Vector3(0.0f, -9.8f, 0.0f);
    const float cor = 0.5f; // Coefficient Of Restitution
    const float groundHeight = -0.8f;

    for (auto &p : m_particlesCPU) {

        if (p.life < 0.0f) // 수명이 다했다면 무시
            continue;

        p.velocity = p.velocity + gravity * dt;
        p.position += p.velocity * dt;
        p.life -= dt;

        if (p.position.y < groundHeight && p.velocity.y < 0.0f) {
            // TODO: ...
        }

        // TODO: ...
    }

    D3D11Utils::CopyToStagingBuffer(
        m_context, m_particlesStagingGPU,
        UINT(sizeof(Particle) * m_particlesCPU.size()), m_particlesCPU.data());
    m_context->CopyResource(m_particlesGPU.Get(), m_particlesStagingGPU.Get());
}

void Ex1501_ParticleSystem::Render() {

    // Timer timer(m_device);
    // timer.Start(m_context, true);

    DrawSprites();

    // timer.End(m_context);
}

void Ex1501_ParticleSystem::DrawSprites() {

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
    m_context->Draw(UINT(m_particlesCPU.size()), 0);
}

void Ex1501_ParticleSystem::UpdateGUI() {}

} // namespace hlab