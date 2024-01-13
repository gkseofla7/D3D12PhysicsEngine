#include "TileableNoise.hlsli"

RWTexture2D<float4> densityTex : register(u0);

struct Particle
{
    float3 position;
    float3 color;
};

RWStructuredBuffer<Particle> outputParticles : register(u1);

// 가시화를 위해 속도 여러가지로 조절
static float dt = 0.001f; // ConstBuffer로 받아올 수도 있음

float getNoise(float2 uv)
{
    // freq 여러가지로 바꿔보기
    return perlinfbm(float3(uv, 0.0f), 2.0, 1);
}

// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf
float2 getCurl(float2 uv, float2 dx)
{
    // TODO: 완성
    float2 dp;
    dp.x = (getNoise(uv + float2(dx.x, 0)) - getNoise(uv - float2(dx.x, 0))) / dx.x * 0.5;
    dp.y = (getNoise(uv + float2(0, dx.y)) - getNoise(uv - float2(0, dx.y))) / dx.y * 0.5;
    
    return float2(dp.y, -dp.x);
}

[numthreads(32, 32, 1)]
void main(uint3 dtID : SV_DispatchThreadID, uint gIndex : SV_GroupIndex)
{
    uint width, height;
    densityTex.GetDimensions(width, height);
    
    float2 uv = dtID.xy / float2(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);
    
    //densityTex[dtID.xy].xyzw = getNoise(uv) * 1.0f; // 꼬리를 보려면 주석 처리
    
    // 쉐이더 수를 줄이기 위해서 입자들이 width 개라고 가정
    outputParticles[dtID.x].position.xy += getCurl(outputParticles[dtID.x].position.xy, dx) * dt;
}

// 쉐이더 토이 참고해서 비슷한 효과 만들어 보기