#include "Common.hlsli" // ���̴������� include ��� ����

//struct DepthOnlyPixelShaderInput
//{
//    float4 posProj : SV_POSITION;
//    float3 posWorld : POSITION0; // World position (���� ��꿡 ���)
//};

//void main(float4 pos : SV_POSITION)
//{
//    // �ƹ��͵� ���� ���� (Depth Only)
//}

float4 main(PixelShaderInput input) : SV_Target
{
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    float4 viewPos = mul(pos, viewProj);
    return float4(viewPos.z / viewPos.w, 0.0, 0.0, 0.0);
}

/* ��: ��ȯ �ڷ��� ���ʿ�
float4 main(float4 pos : SV_POSITION) : SV_Target0 
{
    return float4(1, 1, 1, 1);
}*/
