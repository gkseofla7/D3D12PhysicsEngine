struct PSInput // GS가 있다면 GSInput으로 사용됨
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

struct Particle
{
    float3 position;
    float3 color;
};

StructuredBuffer<Particle> particles : register(t0);

// VSInput이 없이 vertexID만 사용
PSInput main(uint vertexID : SV_VertexID)
{
    Particle p = particles[vertexID];
    
    PSInput output;
    
    output.position = float4(p.position.xyz, 1.0);
    
    output.color = p.color;

    return output;
}
