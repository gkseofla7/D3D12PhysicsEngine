// Advect Velocity and Density

Texture2D<float2> velocityTemp : register(t0);
Texture2D<float4> densityTemp : register(t1);

RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);

// Repeated Boundary
SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

cbuffer Consts : register(b0)
{
    float dt;
    float viscosity;
    float alpha;
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    velocity.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);
    float2 pos = (dtID.xy + 0.5) * dx; // 이 쓰레드가 처리하는 셀의 중심
    
    // TODO: 1. velocityTemp로부터 속도 샘플링해오기
    float2 vel = float2(1.0, 0);
    
    // TODO: 2. 그 속도를 이용해서 역추적 위치 계산
    // float2 posBack = ...;

    // TODO: 3. 그 위치에서 샘플링 해오기
    // velocity[dtID.xy] = ...;
    // density[dtID.xy] = ...;
}
