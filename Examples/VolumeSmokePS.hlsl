#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

#define PI 3.141592

Texture3D<float> densityTex : register(t5); // t5 부터 시작
Texture3D<float> lightingTex : register(t6);
Texture3D<float> temperatureTex : register(t7);

cbuffer Consts : register(b3) // b3 주의
{
    float3 uvwOffset; // 미사용
    float lightAbsorptionCoeff = 5.0;
    float3 lightDir = float3(0, 1, 0);
    float densityAbsorption = 10.0;
    float3 lightColor = float3(1, 1, 1) * 40.0;
    float aniso = 0.3;
}

// 박스 가장자리 좌표로부터 3D 텍스춰 좌표 계산
float3 GetUVW(float3 posModel)
{
    return (posModel.xyz + 1.0) * 0.5;
}

// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-2.html
float BeerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

// Henyey-Greenstein phase function
// Graph: https://www.researchgate.net/figure/Henyey-Greenstein-phase-function-as-a-function-of-O-O-for-isotropic-scattering-g_fig1_338086693
float HenyeyGreensteinPhase(in float3 L, in float3 V, in float aniso)
{
    // V: eye - pos 
    // L: 조명을 향하는 방향
    // https://www.shadertoy.com/view/7s3SRH
    
    float cosT = dot(L, -V);
    float g = aniso;
    return (1.0 - g * g) / (4.0 * PI * pow(abs(1.0 + g * g - 2.0 * g * cosT), 3.0 / 2.0));
}

// https://github.com/maruel/temperature/blob/master/temperature.go
float3 ToRGB(float kelvin)
{
    if (kelvin == 6500.0f)
    {
        return float3(1.0f, 1.0f, 1.0f);
    }

    float temperature = kelvin * 0.01f;
    if (kelvin < 6500.0f)
    {
        float b = 0.0;
        float r = 1.0;
        float green = temperature - 2.0;
        float g = (-155.25485562709179f - 0.44596950469579133f * green + 104.49216199393888f * log(green));

        if (kelvin > 2000.0f)
        {
            float blue = temperature - 10.0f;
            b = (-254.76935184120902f + 0.8274096064007395f * blue + 115.67994401066147f * log(blue)) * 255.0f;
        }
        return float3(r, g, b);
    }

    float b = 1.0f;
    float red = temperature - 55.0f;
    float r = (351.97690566805693f + 0.114206453784165f * red - 40.25366309332127f * log(red));
    float green = temperature - 50.0f;
    float g = (325.4494125711974f + 0.07943456536662342f * green - 28.0852963507957f * log(green)) * 255.0f;
    return float3(r, g, b);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 eyeModel = mul(float4(eyeWorld, 1), worldInv).xyz; // 월드->모델 역변환
    float3 dirModel = normalize(input.posModel - eyeModel);
    
    int numSteps = 128;
    float stepSize = 2.0 / float(numSteps); // 박스 길이가 2.0
    
    // float absorptionCoeff = 10.0;
    float3 volumeAlbedo = float3(1, 1, 1);
    // float3 lightColor = float3(1, 1, 1) * 40.0;
    
    float4 color = float4(0, 0, 0, 1); // visibility 1.0으로 시작
    float3 posModel = input.posModel + dirModel * 1e-6; // 살짝 들어간 상태에서 시작

    // 주의: color.a에 "투명도"로 사용하다가 마지막에 "불투명도"로 바꿔줌
    
    [loop] // [unroll] 사용 시 쉐이더 생성이 너무 느림
    for (int i = 0; i < numSteps; i++)
    {
        float3 uvw = GetUVW(posModel); // +uvwOffset; 미사용 

        // 물체 렌더링
        /*{
            float3 objCenter = float3(0.15, 0.3, 0.5);
            float objRadius = 0.06;
            float dist = length((uvw - objCenter) * float3(2, 1, 1)) / objRadius;
    
            if (dist < 1.0)
            {
                color.rgb += float3(0, 0, 1) * color.a; // Blue ball
                color.a = 0;
            
                // 참고: 물체들을 레스터화로 먼저 렌더링 하고 깊이를 참고해서 블렌딩할 수도 있습니다.
            
                break;
            }
        }*/
        
        float density = densityTex.SampleLevel(linearClampSampler, uvw, 0).r;
        // float lighting = lightingTex.SampleLevel(linearClampSampler, uvw, 0).r;
        float lighting = 1.0; // 라이트맵이 없는 예제

        if (density.r > 1e-3)
        {
            float prevAlpha = color.a;
            color.a *= BeerLambert(densityAbsorption * density.r, stepSize);
            float absorptionFromMarch = prevAlpha - color.a;
            
            color.rgb += absorptionFromMarch * volumeAlbedo * lightColor
                         * density * lighting
                         * HenyeyGreensteinPhase(lightDir, dirModel, aniso);
        }
        
        posModel += dirModel * stepSize;
        
        if (abs(posModel.x) > 1 || abs(posModel.y) > 1 || abs(posModel.z) > 1)
            break;
        
        if (color.a < 1e-3)
            break;

    }

    color = saturate(color);
    color.a = 1.0 - color.a; // a는 불투명도
    
    return color;
}
