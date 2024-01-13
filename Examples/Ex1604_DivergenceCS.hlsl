Texture3D<float4> velocity : register(t0);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
Texture3D<int> bc : register(t2);

RWTexture3D<float> divergence : register(u0);
RWTexture3D<float> pressure : register(u1);
RWTexture3D<float> pressureTemp : register(u2);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
}

static int3 offset[6] =
{
    int3(1, 0, 0), // right
    int3(-1, 0, 0), // left
    int3(0, 1, 0), // up
    int3(0, -1, 0), // down
    int3(0, 0, 1), // back
    int3(0, 0, -1) // front
};

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    if (bc[dtID.xyz] >= 0) // 가장자리 조건 주의
    {
        float div = 0.0;

        [unroll]
        for (int i = 0; i < 6; i++)
        {
            if (bc[dtID.xyz + offset[i]] == -1) // Dirichlet
            {
                div += dot(velocity[dtID.xyz].xyz, float3(offset[i]));
            }
            if (bc[dtID.xyz + offset[i]] == -2) // Neumann
            {
                // TODO:
                // div += ...;                
            }
            else
            {
                // TODO:
                // div += ...;                
            }
        }

        divergence[dtID.xyz] = 0.5 * div;
        pressure[dtID.xyz] = 0.0;
        pressureTemp[dtID.xyz] = 0.0;
    }
}
