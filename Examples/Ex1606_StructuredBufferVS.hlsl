#include "Common.hlsli"
#include "Ex1606_Common.hlsli"

struct PSInput // GS가 있다면 GSInput으로 사용됨
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION0;
};

StructuredBuffer<Particle> particles : register(t0);
StructuredBuffer<SortElement> sortElements : register(t1);

// VSInput이 없이 vertexID만 사용
PSInput main(uint vertexID : SV_VertexID)
{
    SortElement e = sortElements[vertexID];
    
    Particle p = particles[e.value];
    
    // 입자 좌표계는 [0,width]x[...]x[...]  라서 object space로 보정
    p.pos.xyz *= dxBase * 2.0;
    p.pos.xyz -= 1.0;
    
    PSInput output;
    
    output.posModel = p.pos.xyz;
    output.posProj = mul(mul(float4(p.pos.xyz, 1.0), world), viewProj);

    return output;
}
