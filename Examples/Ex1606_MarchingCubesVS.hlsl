#include "Common.hlsli"
#include "Ex1606_Common.hlsli"

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
};

GeometryShaderInput main(uint vertexID : SV_VertexID)
{
    GeometryShaderInput output;
    
    output.pos = float4(0, 0, 0, 0); // dummy
    
    return output;
}