#include "Ex1606_Common.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);
RWStructuredBuffer<SortElement> sortElements : register(u1);
RWStructuredBuffer<uint> numActiveParticles : register(u2);
RWTexture3D<uint> firstIndex : register(u3);

uint3 Index3(in uint i1)
{
    uint width, height, depth;
    firstIndex.GetDimensions(width, height, depth);
    
    uint3 i3;
    i3.z = i1 / (width * height);
    i1 -= i3.z * (width * height);
    i3.y = i1 / width;
    i1 -= i3.y * width;
    i3.x = i1;
    
    return i3;
}

[numthreads(1024, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint numStructs, stride;
    sortElements.GetDimensions(numStructs, stride);
    
    if (dtID.x < numStructs && sortElements[dtID.x].key != INACTIVE) // active particles
    {
        // Store number of particles
        if (dtID.x == numStructs - 1)
        {
            // all particles are active
            numActiveParticles[0] = numStructs;
        }
        else if (sortElements[dtID.x + 1].key == INACTIVE)
        {
            // This is the last active one
            numActiveParticles[0] = dtID.x + 1;
        }
        
        if (dtID.x == 0 || sortElements[dtID.x - 1].key != sortElements[dtID.x].key)
        {
            // This is the first particle in the cell
            uint3 i3 = Index3(sortElements[dtID.x].key);
            firstIndex[i3] = dtID.x;
        }
    }
}
