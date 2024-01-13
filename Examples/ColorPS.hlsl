#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.pixelColor.rgba = 1.0;
    
    return output;
}
