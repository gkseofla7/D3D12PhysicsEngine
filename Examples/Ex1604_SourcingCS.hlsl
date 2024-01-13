RWTexture3D<float4> velocity : register(u0);
RWTexture3D<float> density : register(u1);

// boundary conditions
// -1: Dirichlet condition
// -2: Neumann condition
//  0: Full cell
RWTexture3D<int> bc : register(u2);

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
    int upScale = 1;
    int numNewParticles = 0;
    float turbulence = 0.0;
    float sourceStrength = 1.0;
    float buoyancy = 0.0;
}

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint width, height, depth;
    bc.GetDimensions(width, height, depth);
    
    bc[dtID] = 0;
    
    if (dtID.x == 0 || dtID.y == 0 || dtID.z == 0
        || dtID.x == width - 1 || dtID.y == height - 1 || dtID.z == depth - 1)
    {
        bc[dtID] = -1; // Dirichlet boundary condition
        density[dtID.xyz] = 0.0;
    }

    // Source
    float3 center = float3(0.02, 0.3, 0.5) / dxBase;
    int radius = 0.2 * height;

    float dist = length(float3(dtID.xyz) - center) / radius;
    
    if (dist < 1.0)
    {
        velocity[dtID.xyz] = float4(32 * sourceStrength, 0, 0, 0) / 64.0 * float(width); // scale up velocity
        density[dtID.xyz] = max(smootherstep(1.0 - dist), density[dtID.xyz]);
        //bc[dtID.xyz] = -2; // Neumann
    }

    // Object
    center = float3(0.15, 0.3, 0.5) / dxBase;
    radius = 0.1 * height;
    
    dist = length(float3(dtID.xyz) - center) / radius;
    
    if (dist < 1.0)
    {
        velocity[dtID.xyz] = float4(0, 0, 0, 0) / 64.0 * width; // ¸ØÃçÀÖ´Â ¹°Ã¼
        // density[dtID.xyz] = 0.0;
        bc[dtID.xyz] = -2; // Neumann
    }

    // buoyancy
    velocity[dtID.xyz] += float4(0, buoyancy, 0, 0) * density[dtID.xyz] * dt * width;
}
