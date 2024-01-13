Texture2D<float> pressureTemp : register(t0);
Texture2D<float> divergence : register(t1);

RWTexture2D<float> pressureOut : register(u0);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    // Dirichlet boundary condition
    if (dtID.x == 0 && dtID.y == 0)
    {
        pressureOut[dtID.xy] = 0.0;
        return;
    }
    
    uint width, height;
    pressureOut.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);
        
    uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
    uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
    uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
    uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
    
    // pressureOut[dtID.xy] = ...;
}
