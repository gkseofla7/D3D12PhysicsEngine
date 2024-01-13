Texture2D<float4> matTex : register(t0); // (width, height) = (numCols/4, numRows/4)
Texture2D<float4> vecTex : register(t1); // (width, height) = (numCols/4, 1)
RWTexture2D<float> outputTex : register(u0); // (width, height) = (numRows/4, 1)

// 출력 벡터의 차원(numRows / 4)에 대해 여러 쓰레드로 나눠서 계산

// 디버깅할 때는 (1, 1, 1)로도 테스트
[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    matTex.GetDimensions(width, height);

    uint r = dtID.x;
    float sum = 0.0;
    for (uint i = 0; i < width; i++)
    {
        // TODO:
    }

    // outputTex[dtID.xy].r = sum; // 행렬 벡터 곱하기

    outputTex[dtID.xy].r += sum; // 반복 누적 테스트
}
