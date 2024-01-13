#define INACTIVE 4294967295U // 0xFFFFFFFF

struct Particle
{
    float3 pos;
    float3 vel;
};

struct SortElement
{
    uint key; // Cell index
    uint value; // Particle index
};

static int3 offset[6] =
{
    int3(1, 0, 0), // right
    int3(-1, 0, 0), // left
    int3(0, 1, 0), // up
    int3(0, -1, 0), // down
    int3(0, 0, 1), // back
    int3(0, 0, -1) // front
};

cbuffer Consts : register(b4)
{
    float3 dxBase;
    float dt;
    float3 dxUp;
    float time;
    int upScale = 1;
    int numNewParticles = 0;
    float turbulence = 0.0;
    float sourceStrength = 1.0;
    uint width;
    uint height;
    uint depth;
}