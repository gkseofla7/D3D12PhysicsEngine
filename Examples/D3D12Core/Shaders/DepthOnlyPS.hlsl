#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

//struct DepthOnlyPixelShaderInput
//{
//    float4 posProj : SV_POSITION;
//    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
//};

//void main(float4 pos : SV_POSITION)
//{
//    // 아무것도 하지 않음 (Depth Only)
//}

float4 main(PixelShaderInput input) : SV_Target
{
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    float4 viewPos = mul(pos, viewProj);
    return float4(viewPos.z / viewPos.w, 0.0, 0.0, 0.0);
}

/* 비교: 반환 자료형 불필요
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
