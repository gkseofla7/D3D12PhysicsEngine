// Advect Velocity and Density

// Low-res
Texture3D<float4> velocityOld : register(t0);
Texture3D<float4> velocityNew : register(t1);
Texture3D<float> densityOld : register(t2);
Texture3D<float> densityNew : register(t3);

// High-res
RWTexture3D<float4> velocityUp : register(u0);
RWTexture3D<float> densityUp : register(u1);

SamplerState pointClampSS : register(s0);
SamplerState linearClampSS : register(s1);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
}

// Run with Up-resolution
[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    float3 uvw = (dtID + 0.5) * dxUp;
    
    float coeff = 0.99; // 0.0: use interpolated from low-res, 1.0: fully diff-upsample
    
    float4 velOld = velocityOld.SampleLevel(linearClampSS, uvw, 0);
    float4 velNew = velocityNew.SampleLevel(linearClampSS, uvw, 0);
    // TODO:
    // velocityUp[dtID] = ...
    
    float denOld = densityOld.SampleLevel(linearClampSS, uvw, 0);
    float denNew = densityNew.SampleLevel(linearClampSS, uvw, 0);
    // TODO:
    // densityUp[dtID] = ...    
}
