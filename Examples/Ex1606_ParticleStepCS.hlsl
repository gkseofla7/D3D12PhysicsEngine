#include "Ex1606_Common.hlsli"

Texture3D<float4> noiseTex : register(t0);
StructuredBuffer<uint> numActiveParticles : register(t1);

RWStructuredBuffer<Particle> particles : register(u0);
RWStructuredBuffer<SortElement> sortElements : register(u1);

bool IsInside(float3 uvw)
{
    if (uvw.x <= dxBase.x)
        return false;
    else if (uvw.y <= dxBase.y)
        return false;
    else if (uvw.z <= dxBase.z)
        return false;
    else if (uvw.x >= 1.0 - dxBase.x)
        return false;
    else if (uvw.y >= 1.0 - dxBase.y)
        return false;
    else if (uvw.z >= 1.0 - dxBase.z)
        return false;
    else
        return true;
}

float3 SphericalToCartesial(in float3 sph)
{
    // x: phi, y: theta, z: radius
    sph.xy *= 3.141592 * 2.0; // 0~360 degrees
    
    return float3(sin(sph.x) * cos(sph.y), sin(sph.x) * sin(sph.y), cos(sph.x)) * sph.z;
}

uint MinCornerCell(float3 particlePos)
{
    int3 i3 = int3(floor(particlePos - 0.5));
    return i3.x + i3.y * width + i3.z * width * height;
}

[numthreads(1024, 1, 1)]
void main(uint3 gId : SV_GroupID, uint3 dtID : SV_DispatchThreadID)
{
    SortElement ref = sortElements[dtID.x];
    
    if (ref.key != INACTIVE) // Active particle
    {
        Particle p = particles[ref.value];
   
        float3 uvw = p.pos.xyz * dxBase;

        // 시뮬레이션 공간 밖으로 나가지 않았을 경우
        if (IsInside(uvw))
        {
            // 속도의 기준은 BaseGrid이지만 입자 좌표계는 UpGrid라서 upScale을 곱해줍니다.
            p.vel += float3(0, -0.2, 0) * dt * float3(width, height, depth); // 중력
            p.pos += p.vel * dt;

            // Object collision
            /*float3 objCenter = float3(0.15, 0.3, 0.5) / dxBase;
            float objRadius = 0.1 / dxBase.y;
            float sqrDist = dot((p.pos - objCenter), (p.pos - objCenter)) - objRadius * objRadius;
            if (sqrDist <= 0.0)
            {
                float dist = sqrt(sqrDist);
                p.pos = objCenter + (p.pos - objCenter) / dist * (objRadius + 1e-2);
            }*/

            sortElements[dtID.x].key = MinCornerCell(particles[ref.value].pos);
            particles[ref.value] = p;
        }
        else
        {
            sortElements[dtID.x].key = INACTIVE;
        }
    }
    else if (dtID.x < numActiveParticles[0] + numNewParticles)
    {
        // Add a new particle

        uint width, height, depth;
        noiseTex.GetDimensions(width, height, depth);

        // Source 1 
        if (dtID.x % 2 == 0)
        {
            float3 sourceCenter = float3(0.02, 0.6, 0.5) / dxBase;
            float sourceRadius = 0.1 / dxBase.y;
            float3 randomPos = noiseTex[uint3(int(time * 654.321) % width, int(time * 123 + dtID.x * 1.73) % height, (time * 456 + gId.x) % depth)].xyz;
            randomPos = randomPos * 2 - 1;
            float radius = sqrt(dot(randomPos, randomPos));
            if (radius <= 1)
            {
                float3 pos = sourceRadius * randomPos + sourceCenter;

                if (pos.x >= 0.5)
                {
                    particles[ref.value].pos = pos;
                    particles[ref.value].vel = float3(32 * sourceStrength / 64.0, 0, 0) / dxBase;
                    sortElements[dtID.x].key = MinCornerCell(particles[ref.value].pos);
                }
            }
        }
        else // Source 2
        {
            float3 sourceCenter = float3(1.0 - 0.02, 0.6, 0.5) / dxBase;
            float sourceRadius = 0.1 / dxBase.y;
            float3 randomPos = noiseTex[uint3(int(time * 654.321) % width, int(time * 123 + dtID.x * 1.73) % height, (time * 456 + gId.x * 7) % depth)].xyz;
            randomPos = randomPos * 2 - 1;
            float radius = sqrt(dot(randomPos, randomPos));
            if (radius <= 1)
            {
                float3 pos = sourceRadius * randomPos + sourceCenter;

                if (pos.x < 1.0 / dxBase.x - 0.5)
                {
                    particles[ref.value].pos = pos;
                    particles[ref.value].vel = float3(-32 * sourceStrength / 64.0, 0, 0) / dxBase;
                    sortElements[dtID.x].key = MinCornerCell(particles[ref.value].pos);
                }
            }
        }
    }
}
