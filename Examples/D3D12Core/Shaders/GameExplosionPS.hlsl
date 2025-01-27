cbuffer BillboardModelConstantData : register(b1)
{
    float3 eyeWorld;
    float width;
    Matrix model; // For vertex shader
    Matrix view; // For vertex shader
    Matrix proj; // For vertex shader
    float objectTime;
    float3 padding;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float4 center : POSITION1;
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

Texture2DArray g_texArray : register(t5);
SamplerState g_sampler : register(s0);

// Game Explosion
// https://www.shadertoy.com/view/XtVyDz

#define loop
static const int layers = 128;
static const float blur = .1;
static const float speed = 4.;
static const float peaks = 8.;
static const float peakStrength = .1;
static const float ringSpeed = 1.5;
static const float smoke = .4;
static const float smokeTime = 40.;

float hash(float seed)
{
    return frac(sin(seed * 3602.64325) * 051.63891);
}

float circle(float radius, float2 pos)
{
    return radius - pos.x;
}


struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float time = objectTime * 1.5;
//#ifdef loop
//    time = fmod(time, 5.);
//#endif
    
    float2 uv = (input.texCoord * 2.0 - 1) * 4;
    float2 puv = float2(length(uv), atan2(uv.x, uv.y)); //polar coordinates

    float3 col = float3(0, 0, 0);
    for (int i = 0; i < layers; i++)
    {
        float prog = float(i) / float(layers);
        float radius = prog * ((1. - 1. / pow(abs(time * speed), 1. / 3.)) * 2.); //decrease radius using cubed
        radius += sin(puv.y * peaks + hash(prog) * 513.) * peakStrength; //modulate radius so it isnt enitly symetrical
        float3 color = float3(
            1. / radius,
            .25,
            .1 * (2. - radius)
        ) / time / abs(log(time * ringSpeed) - puv.x); // base explosion color, decrease with time and with distance from center
        float3 temp;
        temp.rgb = (1. - time / smokeTime) * puv.x * smoke;
        color += temp; //add smoke color, falloff can be controlled with smoketime variable
        col += color * smoothstep(0., 1., circle(radius, puv) / blur);
    }

    col /= float(layers);
    
    PixelShaderOutput output;
    float alpha = saturate(smoothstep(dot(float3(1, 1, 1), col), 0, 1));
    output.pixelColor = float4(col.rgb, alpha);
    return output;
}

