Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

cbuffer SamplingPixelConstantData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float4 options;
};

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 FilmicToneMapping(float3 color)
{
    color = max(float3(0, 0, 0), color);
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

//float3 LinearToneMapping(float3 color)
//{
//    float3 invGamma = float3(1, 1, 1) / gamma;

//    color = clamp(exposure * color, 0., 1.);
//    color = pow(color, invGamma);
//    return color;
//}

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    float3 color = g_texture0.Sample(g_sampler, input.texcoord).rgb;
    color = FilmicToneMapping(color);
    //color = float3(1, 1, 1);
    return float4(color, 1.0f);
    
    //float l = (color.x + color.y + color.y) / 3;
   
    //if (l > threshold)
    //{
    //    return float4(color, 1.0f);
    //}
    //else
    //{
    //    return float4(0.0f, 0.0f, 0.0f, 0.0f);
    //}
}