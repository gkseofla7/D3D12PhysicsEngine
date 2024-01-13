struct Particle
{
    float3 pos;
    float3 color;
};

static float dt = 1 / 60.0; // ConstBuffer로 받아올 수 있음

//StructuredBuffer<Particle> inputParticles : register(t0); // SRV로 사용 가능
//RWStructuredBuffer<Particle> outputParticles : register(u0);

ConsumeStructuredBuffer<Particle> inputParticles : register(u0);
AppendStructuredBuffer<Particle> outputParticles : register(u1);

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    Particle p = inputParticles.Consume(); // Read
    
    float3 velocity = float3(-p.pos.y, p.pos.x, 0.0) * 0.1;
    p.pos += velocity * dt;
    
    outputParticles.Append(p); // Write
}
