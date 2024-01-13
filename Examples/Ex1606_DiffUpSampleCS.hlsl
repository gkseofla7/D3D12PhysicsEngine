#include "Common.hlsli"
#include "Ex1606_Common.hlsli"

Texture3D<float4> velocityOld : register(t0);
Texture3D<float4> velocityNew : register(t1);
Texture3D<float> densityNew : register(t2);
Texture3D<float> signedDistance : register(t3);

RWStructuredBuffer<Particle> particles : register(u0);
RWStructuredBuffer<SortElement> sortElements : register(u1);

[numthreads(1024, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    float coeff = 0.97; // 0.0: use interpolated from low-res, 1.0: fully diff-upsample
    
    SortElement ref = sortElements[dtID.x];
    
    float3 uvw = particles[ref.value].pos.xyz * dxBase;
    float d = densityNew.SampleLevel(linearClampSampler, uvw, 0).r;
    float s = signedDistance.SampleLevel(linearClampSampler, uvw, 0).r;
    
    if (d > 1e-5 && s < -0.1 && ref.key != INACTIVE) // Active particle
    {
        float3 velOld = velocityOld.SampleLevel(linearClampSampler, uvw, 0).xyz;
        float3 velNew = velocityNew.SampleLevel(linearClampSampler, uvw, 0).xyz;
        
        particles[ref.value].vel = lerp(velNew, particles[ref.value].vel + velNew - velOld, coeff);
    }
}
