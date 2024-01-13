Texture2D spriteTex : register(t0);
SamplerState linearWrapSampler : register(s1);

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // not POSITION
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    uint primID : SV_PrimitiveID;
};

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

// 일반적으로 Sprite는 텍스춰를 많이 사용합니다.
// 이 예제처럼 수식으로 패턴을 만들 수도 있습니다.
float4 main(PixelShaderInput input) : SV_TARGET
{
    //float dist = length(float2(0.5, 0.5) - input.texCoord) * 2;
    //float scale = smootherstep(1 - dist);
    //return float4(input.color.rgb * scale, 1);
    
    float2 uv = input.texCoord;
    if (input.primID % 4 == 0 || input.primID % 4 == 2)
    {
        uv.x -= 0.5;
        uv.x = -uv.x;
        uv.x += 0.5;
    }
    if (input.primID % 4 == 1 || input.primID % 4 == 2)
    {
        uv.y -= 0.5;
        uv.y = -uv.y;
        uv.y += 0.5;
    }
    
    float4 sprite = spriteTex.Sample(linearWrapSampler, uv);

    return float4(input.color.rgb * sprite.rgb * sprite.a * 0.5, 1);
}
