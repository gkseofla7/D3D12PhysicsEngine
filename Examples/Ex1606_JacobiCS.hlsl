#include "Ex1606_Common.hlsli"

Texture3D<float> pressureTemp : register(t0);
Texture3D<float> divergence : register(t1);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
Texture3D<int> bc : register(t2);

RWTexture3D<float> pressure : register(u0);

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    if (bc[dtID.xyz] >= 0)
    {
        float temp = 0.0;

        //for (int i = 0; i < 6; i++)
        //  temp += pressureTemp[dtID.xyz + offset[i]];
        
        [unroll]
        for (int i = 0; i < 6; i++)
        {
            if (bc[dtID.xyz + offset[i]] == -1) // Dirichlet
            {
                temp += -pressure[dtID.xyz];
            }
            else if (bc[dtID.xyz + offset[i]] == -2) // Neumann
            {
                temp += pressure[dtID.xyz];
            }
            else
                temp += pressure[dtID.xyz + offset[i]];
        }

        pressure[dtID.xyz] = (-divergence[dtID.xyz] + temp) / 6.0;
    }
}
