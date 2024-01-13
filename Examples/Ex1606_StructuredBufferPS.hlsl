#include "Ex1606_Common.hlsli"
#include "Common.hlsli"

Texture3D<float> signedDistance : register(t5); // t5 부터 시작

struct PSInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION0;
};

float4 ComputeNormal(float3 uvw)
{
    float3 normal;
    normal.x = signedDistance.SampleLevel(linearClampSampler, uvw + float3(dxBase.x * 0.5, 0, 0), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(dxBase.x * 0.5, 0, 0), 0);
    normal.y = signedDistance.SampleLevel(linearClampSampler, uvw + float3(0, dxBase.y * 0.5, 0), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(0, dxBase.y * 0.5, 0), 0);
    normal.z = signedDistance.SampleLevel(linearClampSampler, uvw + float3(0, 0, dxBase.z * 0.5), 0)
               - signedDistance.SampleLevel(linearClampSampler, uvw - float3(0, 0, dxBase.z) * 0.5, 0);
    return float4(normalize(normal), 0);
}

float4 main(PSInput input) : SV_TARGET
{
    float3 eyeModel = mul(float4(eyeWorld, 1), worldInv).xyz; // 월드->모델 역변환
    float3 dirModel = normalize(input.posModel - eyeModel);
    
    float3 uvw = (input.posModel.xyz + 1.0) * 0.5;
    float3 normal = ComputeNormal(uvw).xyz;
            
    float3 lightDir = normalize(float3(0, 1, 0) + -dirModel); // 조명 방향을 살짝 시점 쪽으로
    
    float4 color = float4(0.8, 0.8, 0.8, 1);
    color.rgb += saturate(dot(normal, lightDir)) * 0.8; // 간단한 Diffuse 조명

    //return float4(1, 1, 1, 1);
    return color;
}