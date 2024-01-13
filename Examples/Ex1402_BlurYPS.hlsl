Texture2D inputTex : register(t0);
SamplerState pointClampSS : register(s0);

static const float weights[11] =
{
    0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
};

static const int blurRadius = 5;

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    uint width, height;
    inputTex.GetDimensions(width, height);
    
    const float2 dy = float2(0.0, 1.0 / height);
    
    float3 blurColor = float3(0, 0, 0);
    
    [unroll]
    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        // No need to clamp index manually thanks to the sampler
        
        float3 inputColor = inputTex.SampleLevel(pointClampSS, input.texcoord + float(i) * dy, 0.0).rgb;
        blurColor += weights[i + blurRadius] * inputColor;
    }
    
    return float4(blurColor, 1.0f);
}