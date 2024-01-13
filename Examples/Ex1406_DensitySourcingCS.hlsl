struct Particle
{
    float3 pos;
    float3 color;
};

static float dt = 1 / 60.0; // ConstBuffer로 받아올 수 있음

static int radius = 10; // 픽셀 단위 반지름

RWStructuredBuffer<Particle> outputParticles : register(u0);
RWTexture2D<float4> densityOutput : register(u1);

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    Particle p = outputParticles[dtID.x]; // Read
    
    float3 velocity = float3(-p.pos.y, p.pos.x, 0.0) * 0.5;

    p.pos += velocity * dt;
    
    outputParticles[dtID.x].pos = p.pos; // Write

    /*
    uint width, height;
    densityOutput.GetDimensions(width, height);
    
    float2 posScreen = float2(p.pos.x, -p.pos.y);
    posScreen = (posScreen + 1) * 0.5;
    posScreen *= float2(width, height);
    posScreen -= 0.5;

    // 출력에 임의 접근 가능 (Bounding Box)
    // 주의: ThreadSafe 하지 않음
    for (int j = max(0, posScreen.y - radius); j <= min(height - 1, posScreen.y + radius); j++)
        for (int i = max(0, posScreen.x - radius); i <= min(width - 1, posScreen.x + radius); i++)
        {
            int2 idx = uint2(i, j);
            idx.x = clamp(idx.x, 0, width - 1);
            idx.y = clamp(idx.y, 0, height - 1);

            float dist = length(idx - posScreen);
            float density = smootherstep(radius - dist, 0.0, radius);

            densityOutput[idx] += float4(p.color.rgb * density, 1.0) * 0.5; // Write
        }
    */
}
