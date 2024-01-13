Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);

// 참고 자료: Luna DX11 교재 Ch. 12

// https://github.com/Microsoft/DirectX-Graphics-Samples/issues/140
// GroupMemoryBarrier - Wait for all issued reads and writes from groupshared memory (LDS) to finish
// DeviceMemoryBarrier - Same but for system/video memory
// AllMemoryBarrier - Flush any and all memory transactions (Group + Device)

static const float weights[11] =
{
    0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
};

static const int blurRadius = 5;

#define N 256
#define CACHE_SIZE (N + 2*blurRadius)

// Groupshared memory is limited to 16KB per group.
// A single thread is limited to a 256 byte region of groupshared memory for writing.
// https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader

groupshared float4 groupCache[CACHE_SIZE];

[numthreads(1, N, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    outputTex.GetDimensions(width, height);

    if (gtID.y < blurRadius)
    {
        int y = max(int(dtID.y) - blurRadius, 0);
        groupCache[gtID.y] = inputTex[int2(dtID.x, y)];
    }
    
    if (gtID.y >= N - blurRadius)
    {
        int y = min(dtID.y + blurRadius, height - 1);
        groupCache[gtID.y + 2 * blurRadius] = inputTex[int2(dtID.x, y)];
    }
    
    groupCache[gtID.y + blurRadius] =
        inputTex[min(dtID.xy, uint2(width, height) - 1)];

    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = float4(0, 0, 0, 0);

    //[unroll]
    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        int k = gtID.y + blurRadius + i;
        blurColor += weights[i + blurRadius] * groupCache[k];
    }

    outputTex[dtID.xy] = blurColor;
}
