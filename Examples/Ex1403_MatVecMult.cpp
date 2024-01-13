#include "Ex1403_MatVecMult.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "Texture2D.h"

#include <random>

namespace hlab {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Ex1403_MatVecMult::Ex1403_MatVecMult() : AppBase() {}

float DotProduct(float *a, float *b, int N) {

    float sum = 0.0f;

    for (int i = 0; i < N; i++) {
        sum += a[i] * b[i];
    }

    return sum;
}

bool Ex1403_MatVecMult::Initialize() {

    cout << "Ex1403_MatVecMult::Initialize()" << endl;

    // ComputeShader에서 BackBuffer를 사용하기 위해서 FLOAT로 설정
    // 이 예제에서는 렌더링하지 않고 계산만 하고 종료
    AppBase::m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    if (!AppBase::Initialize())
        return false;

    m_repeat = 1;
    m_numRows = 1024; //    *16;
    m_numCols = 2048; //    *8;

    PrepareForData();

    TestCPU();

    TestGPU();

    // 7. 결과 비교

    float errorGPUSum = 0.0f;
    for (int i = 0; i < m_myResult.size(); i++) {
        errorGPUSum += abs(m_myResult[i] - m_myResultGPU[i]);

        // 하나하나 다 출력해서 확인해보고 싶은 경우
        // cout << "Diff " << m_myResult[i] << " " << myResultGPU[i] << " "
        //    << abs(m_myResult[i] - myResultGPU[i])
        //    << endl;
    }
    cout << "Error GPU " << errorGPUSum << endl;

    cout << "GPU Result "
         << DotProduct(m_myResultGPU.data(), m_myResultGPU.data(), m_numRows)
         << endl;

    exit(0); // 렌더링 하지 않고 바로 종료

    return true;
}

void Ex1403_MatVecMult::Update(float dt) {}

void Ex1403_MatVecMult::Render() {}

void Ex1403_MatVecMult::PrepareForData() {

    // (m_numRows by m_numCols) Matrix 곱하기 m_numCols Vector
    // -> m_numRows Vector

    m_myMat = vector<float>(m_numRows * m_numCols);
    m_myVec = vector<float>(m_numCols);
    m_myResult = vector<float>(m_numRows, 0.0f); // 결과 저장용

    // 랜덤 넘버 생성기 (숫자가 클 경우 정밀도 문제 발생 -> floor로 절삭)
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::mt19937 gen(0);
    std::uniform_real_distribution<float> dist(0, 10);
    std::generate(m_myMat.begin(), m_myMat.end(),
                  [&]() { return floor(dist(gen)); });
    std::generate(m_myVec.begin(), m_myVec.end(),
                  [&]() { return floor(dist(gen)); });

    // 디버깅용 데이터 (숫자가 클 경우 정밀도 문제 발생)
    // for (size_t i = 0; i < m_myMat.size(); i++)
    //    m_myMat[i] = float(i % 10);
    // for (int i = 0; i < m_myVec.size(); i++)
    //    m_myVec[i] = float(i % 10);
}

void Ex1403_MatVecMult::TestCPU() {

    Timer timer(m_device);
    timer.Start(m_context, false);

    for (int i = 0; i < m_repeat; i++) {
        size_t offset = 0;
        for (int r = 0; r < m_numRows; r++) {

            // 행렬 벡터 곱하기 테스트
            // m_myResult[r] = DotProduct(&m_myMat[offset], &m_myVec[0],
            // m_numCols));

            // 반복 누적 테스트 용도 (+= 사용)
            m_myResult[r] +=
                DotProduct(&m_myMat[offset], &m_myVec[0], m_numCols);

            offset += m_numCols;
        }
    }

    timer.End(m_context);

    cout << "Result CPU: "
         << DotProduct(m_myResult.data(), m_myResult.data(), m_numRows) << endl;
}

void Ex1403_MatVecMult::TestGPU() {

    // 1. Texture 초기화

    Texture2D matTex, vecTex, outputTex;

    matTex.Initialize(m_device, m_numCols / 4, m_numRows,
                      DXGI_FORMAT_R32G32B32A32_FLOAT);
    vecTex.Initialize(m_device, m_numCols / 4, 1,
                      DXGI_FORMAT_R32G32B32A32_FLOAT);
    outputTex.Initialize(m_device, m_numRows,       // 주의: m_numRows
                         1, DXGI_FORMAT_R32_FLOAT); // 주의: R32

    // 2. 데이터 업로드 (내부적으로 Staging texture 사용)

    // 주의: RGBA 사용으로 m_numCols/4
    std::vector<uint8_t> buffer;

    buffer.resize(m_myMat.size() * sizeof(float) / sizeof(uint8_t));
    memcpy(buffer.data(), m_myMat.data(), buffer.size());

    matTex.Upload(m_device, m_context, buffer);

    buffer.resize(m_myVec.size() * sizeof(float) / sizeof(uint8_t));
    memcpy(buffer.data(), m_myVec.data(), buffer.size());

    vecTex.Upload(m_device, m_context, buffer);

    vector<float> myResultZero(m_numRows, 0.0f);
    buffer.resize(myResultZero.size() * sizeof(float) / sizeof(uint8_t));
    memcpy(buffer.data(), myResultZero.data(), buffer.size());

    outputTex.Upload(m_device, m_context, buffer);

    // 3. 쉐이더 호출
    ComPtr<ID3D11ComputeShader> matVecMultCS;
    D3D11Utils::CreateComputeShader(m_device, L"Ex1403_MatVecMultCS.hlsl",
                                    matVecMultCS);

    Timer timer(m_device);
    timer.Start(m_context, true);

    // 컴퓨터 쉐이더만 사용하는 특별한 상황
    m_context->CSSetShader(matVecMultCS.Get(), 0, 0);

    // 주의: R32라서 numRows를 4로 나눠주지 않음
    for (int i = 0; i < m_repeat; i++) {
        ID3D11ShaderResourceView *srvs[2] = {matTex.GetSRV(), vecTex.GetSRV()};
        m_context->CSSetShaderResources(0, 2, srvs);
        m_context->CSSetUnorderedAccessViews(0, 1, outputTex.GetAddressOfUAV(),
                                             NULL);
        m_context->Dispatch(UINT(ceil(m_numRows / 256.0f)), 1, 1);
        AppBase::ComputeShaderBarrier(); // 이 예제에서는 생략 가능
    }

    /* 실험 결과는 여러 가지 요소에 따라 많이 달라집니다.

    1회 반복
    GPU : 0.064832, CPU : 3.1

    10회 반복
    GPU: 0.521728, CPU: 3.5592 <- CPU 오버헤드가 10배로 증가하지는 않음

    100회 반복
    GPU: 5.09747, CPU: 8.1159 <- CPU 오버헤드가 100배로 증가하지는 않음
    */

    timer.End(m_context);

    // 4. GPU 계산 결과를 CPU로 복사 (중간에 Staging texture 사용)
    buffer.resize(m_numRows * sizeof(float) / sizeof(uint8_t));
    std::fill(buffer.begin(), buffer.end(), 0);

    outputTex.Download(m_context, buffer);

    // uint8_t -> float 복사
    m_myResultGPU = vector<float>(m_myResult.size());
    memcpy(m_myResultGPU.data(), buffer.data(), buffer.size());
}

void Ex1403_MatVecMult::UpdateGUI() {}

} // namespace hlab