struct Particle
{
    float3 pos;
    float3 color;
};

static float dt = 1 / 60.0; // ConstBuffer로 받아올 수 있음

//StructuredBuffer<Particle> inputParticles : register(t0); // SRV로 사용 가능
RWStructuredBuffer<Particle> outputParticles : register(u0);

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    Particle p = outputParticles[dtID.x]; // Read
    
    // float3 velocity = ...;
    // p.pos += ...;
    
    outputParticles[dtID.x].pos = p.pos; // Write
}
