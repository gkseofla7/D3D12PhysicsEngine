// Advect Velocity and Density

Texture3D<float4> velocityTemp : register(t0);
Texture3D<float> densityTemp : register(t1);

RWTexture3D<float4> velocity : register(u0);

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

float3 Vorticity(uint3 center)
{
    // velocity = (v1, v2, v3)
    
    // https://www.khanacademy.org/math/multivariable-calculus/multivariable-derivatives/divergence-and-curl-articles/a/curl#:~:text=Curl%20is%20an%20operator%20which%20measures%20rotation%20in,flow%20indicated%20by%20a%20three%20dimensional%20vector%20field.
    float v3_y = velocityTemp[center + int3(0, 1, 0)].z - velocityTemp[center - int3(0, 1, 0)].z;
    float v2_z = velocityTemp[center + int3(0, 0, 1)].y - velocityTemp[center - int3(0, 0, 1)].y;
    float v1_z = velocityTemp[center + int3(0, 0, 1)].x - velocityTemp[center - int3(0, 0, 1)].x;
    float v3_x = velocityTemp[center + int3(1, 0, 0)].z - velocityTemp[center - int3(1, 0, 0)].z;
    float v2_x = velocityTemp[center + int3(1, 0, 0)].y - velocityTemp[center - int3(1, 0, 0)].y;
    float v1_y = velocityTemp[center + int3(0, 1, 0)].x - velocityTemp[center - int3(0, 1, 0)].x;
    
    return float3(v3_y - v2_z, v1_z - v3_x, v2_x - v1_y) * 0.5;
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    velocityTemp.GetDimensions(width, height, depth);

    float density = densityTemp[dtID.xyz];

    if (density > 1e-2
        && dtID.x > 1 && dtID.y > 1 && dtID.z > 1
        && dtID.x < width - 2 && dtID.y < height - 2 && dtID.z < depth - 2)
    {
        float3 eta;
        eta.x = length(Vorticity(dtID + int3(1, 0, 0))) - length(Vorticity(dtID - int3(1, 0, 0)));
        eta.y = length(Vorticity(dtID + int3(0, 1, 0))) - length(Vorticity(dtID - int3(0, 1, 0)));
        eta.z = length(Vorticity(dtID + int3(0, 0, 1))) - length(Vorticity(dtID - int3(0, 0, 1)));
        
        float l = length(eta);

        if (l > 1e-3)
        {
            float3 N = eta / l;
            velocity[dtID.xyz] += float4(cross(N, Vorticity(dtID.xyz)) * dt
                                * turbulence // 사용자 조절 변수
                                * density // 밀도가 높은 곳에만 적용
                                * depth, // 격자 해상도 보정
                                0);
        }
    }
}
