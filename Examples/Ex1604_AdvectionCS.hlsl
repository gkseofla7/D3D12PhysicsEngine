// Advect Velocity and Density

Texture3D<float4> velocityTemp : register(t0);
Texture3D<float> densityTemp : register(t1);

RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

SamplerState pointClampSS : register(s0);
SamplerState linearClampSS : register(s1);
SamplerState linearMirrorSS : register(s2);
SamplerState pointWrapSS : register(s3);
SamplerState linearWrapSS : register(s4);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
    int upScale;
    int numNewParticles;
    float turbulence;
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    velocity.GetDimensions(width, height, depth);
    float3 dx = float3(1.0 / width, 1.0 / height, 1.0 / depth);
    float3 uvw = (dtID.xyz + 0.5) * dx; // 픽셀 중심

    // TODO:
    // 주의: 샘플링된 속도에 dx를 곱해줘야 함
    // float3 vel = ...;
   
    // TODO:
    // 속도의 기준은 BaseGrid 해상도이고 Advection은 UpGrid에서 하기때문에 
    // 속도에 upScale을 곱해줍니다.
    // float3 uvwBack = ...;
    
    // TODO:
    // density[dtID.xyz] = ...;
    // velocity[dtID.xyz] = ...;
}
