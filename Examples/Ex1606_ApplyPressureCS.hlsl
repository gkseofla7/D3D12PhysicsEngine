Texture3D<float> pressure : register(t0);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
Texture3D<int> bc : register(t2);

RWTexture3D<float4> velocity : register(u0);

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
        float p[6];

        [unroll]
        for (int i = 0; i < 6; i++)
        {
            if (bc[dtID.xyz + offset[i]] == -1) // Dirichlet
            {
                p[i] = -pressure[dtID.xyz];
            }
            else if (bc[dtID.xyz + offset[i]] == -2) // Neumann
            {
                p[i] = pressure[dtID.xyz];
            }
            else
                p[i] = pressure[dtID.xyz + offset[i]];
        }

        velocity[dtID.xyz] -= 0.5 * float4(p[0] - p[1], p[2] - p[3], p[4] - p[5], 0);
    }
}
