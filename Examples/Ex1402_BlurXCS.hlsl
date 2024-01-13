Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);

SamplerState pointClampSS : register(s0); // PS 처럼 샘플러 사용 가능

// 참고 자료: Luna DX11 교재 Ch. 12

static const float weights[11] =
{
    0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
};

static const int blurRadius = 5;

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    //uint width, height;
    //outputTex.GetDimensions(width, height);
    //float2 dx = float2(1.0 / width, 1.0 / height);
    
    float dx = 1.0 / 1280;
    float dy = 1.0 / 768;
    // 또는 float2 dx = float2(1.0 / 1280, 1.0 / 768)
    
    float2 uv = float2((dtID.x + 0.5) * dx, (dtID.y + 0.5) * dy);
    // float2 uv = float2((dtID.x + 0.5) * dx.x, (dtID.y + 0.5) * dx.y);

    float3 blurColor = float3(0, 0, 0);

    [unroll]
    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        //uint clamped = uint(clamp(i + int(dtID.x), 0, int(width) - 1));
        //float3 color = inputTex[uint2(clamped, dtID.y)].xyz;
        
        float3 color = inputTex.SampleLevel(pointClampSS, uv + float2(float(i) * dx, 0.0), 0.0).rgb;

        blurColor += weights[i + blurRadius] * color;
    }

    outputTex[dtID.xy] = float4(blurColor, 1);
}
