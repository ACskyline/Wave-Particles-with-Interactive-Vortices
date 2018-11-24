#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    return t0.Sample(s0, input.texCoord);
}