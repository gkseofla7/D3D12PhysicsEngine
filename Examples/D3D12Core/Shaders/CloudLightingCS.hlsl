Texture3D<float> densityTex : register(t0);
RWTexture3D<float> lightingTex : register(u0);

SamplerState linearClampSampler : register(s1);

cbuffer Consts : register(b0)
{
    float3 uvwOffset;
    float lightAbsorptionCoeff = 5.0;
    float3 lightDir = float3(0, 1, 0);
    float densityAbsorption = 10.0;
    float3 lightColor = float3(1, 1, 1) * 40.0;
    float aniso = 0.3;
}

// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-2.html
float BeerLambert(float absorptionCoefficient, float distanceTraveled)
{
    return exp(-absorptionCoefficient * distanceTraveled);
}

// 박스 가장자리 좌표로부터 3D 텍스춰 좌표 계산
float3 GetUVW(float3 posModel)
{
    return (posModel.xyz + 1.0) * 0.5;
}

float LightRay(float3 posModel, float3 lightDir)
{
    // 근처만 탐색
    int numSteps = 128 / 4;
    float stepSize = 2.0 / float(numSteps);
    // float absorptionCoeff = 5.0;

    float alpha = 1.0; // visibility 1.0으로 시작

    [loop] // [unroll] 사용 시 쉐이더 생성이 너무 느림
    for (int i = 0; i < numSteps; i++)
    {
        float prevAlpha = alpha;
        float density = densityTex.SampleLevel(linearClampSampler, GetUVW(posModel), 0).
        r;
        
        if (density > 1e-3)
            alpha *= BeerLambert(lightAbsorptionCoeff * density, stepSize);

        posModel += lightDir * stepSize;

        if (abs(posModel.x) > 1 || abs(posModel.y) > 1 || abs(posModel.z) > 1)
            break;
        
        if (alpha < 1e-3)
            break;
    }
    
    // alpha가 0에 가까울 수록 조명으로부터 빛을 잘 못 받음
    return alpha;
}

[numthreads(16, 16, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    // float3 lightDir = float3(0, 1, 0);
    
    uint width, height, depth;
    lightingTex.GetDimensions(width, height, depth);
    
    float3 uvw = dtID / float3(width, height, depth); //+ uvwOffset; 라이트맵은 주어진 밀도장에 대해 계산하는 것이라서 uvwOffset 미사용

    // uvw는 [0, 1]x[0,1]x[0,1]
    // 모델 좌표계는 [-1,1]x[-1,1]x[-1,1]
    lightingTex[dtID] = LightRay((uvw - 0.5) * 2.0, lightDir);
}
