#include "Ex1606_Common.hlsli"

Texture3D<float4> velocityTemp : register(t0);

RWTexture3D<float> divergence : register(u0);
RWTexture3D<float> pressure : register(u1);
RWTexture3D<float> pressureTemp : register(u2);
RWTexture3D<float> density : register(u3);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
RWTexture3D<int> bc : register(u4);

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    if (dtID.x == 0 || dtID.y == 0 || dtID.z == 0
        || dtID.x == width - 1 || dtID.y == height - 1 || dtID.z == depth - 1
        || density[dtID] < 1e-6)
    {
        bc[dtID] = -1; // Dirichlet
    }
    else
    {
        bc[dtID] = 0; // 참고: Linear system을 구성할 때는 cell index 대입. 여기서는 0이면 Full cell
        
        float div = 0.0;

        [unroll]
        for (int i = 0; i < 6; i++)
        {
            if (bc[dtID.xyz + offset[i]] == -1) // Dirichlet
                div += dot(velocityTemp[dtID.xyz].xyz, float3(offset[i]));
            if (bc[dtID.xyz + offset[i]] == -2) // Neumann
                div += dot(2 * velocityTemp[dtID.xyz + offset[i]].xyz - velocityTemp[dtID.xyz].xyz, float3(offset[i]));
            else
                div += dot(velocityTemp[dtID.xyz + offset[i]].xyz, float3(offset[i]));
        }

        divergence[dtID.xyz] = 0.5 * div;
        pressure[dtID.xyz] = 0.0;
        pressureTemp[dtID.xyz] = 0.0;
    }
}
