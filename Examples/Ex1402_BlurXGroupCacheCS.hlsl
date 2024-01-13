Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);

// 참고 자료: Luna DX11 교재 Ch. 12

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

[numthreads(N, 1, 1)]
void main(uint3 gID : SV_GroupID, uint3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    outputTex.GetDimensions(width, height);

    if (gtID.x < blurRadius)
    {
        int x = max(int(dtID.x) - blurRadius, 0);
        groupCache[gtID.x] = inputTex[int2(x, dtID.y)];
    }
    
    if (gtID.x >= N - blurRadius)
    {
        int x = min(dtID.x + blurRadius, width - 1);
        groupCache[gtID.x + 2 * blurRadius] = inputTex[int2(x, dtID.y)];
    }
    
    groupCache[gtID.x + blurRadius] =
        inputTex[min(dtID.xy, uint2(width, height) - 1)];

    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = float4(0, 0, 0, 0);

    [unroll]
    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        int k = gtID.x + blurRadius + i;
        blurColor += weights[i + blurRadius] * groupCache[k];
    }

    outputTex[dtID.xy] = blurColor;
}
