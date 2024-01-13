// Advect Velocity and Density

Texture3D<float4> velocityUp : register(t0);
Texture3D<float> densityUp : register(t1);

RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
    int upScale;
}

// Run with Down-resolution
[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    float4 velocitySum = float4(0, 0, 0, 0);
    float densitySum = 0.0;
    
    // [loop]
    for (int k = 0; k < upScale; k++)
        for (int j = 0; j < upScale; j++)
            for (int i = 0; i < upScale; i++)
            {
                // TODO:
                // velocitySum += ...;
                // densitySum += ...; 
            }
    
    float scale = 1.0 / (upScale * upScale * upScale);
    
    velocity[dtID] = velocitySum * scale;
    density[dtID] = densitySum * scale;
}
