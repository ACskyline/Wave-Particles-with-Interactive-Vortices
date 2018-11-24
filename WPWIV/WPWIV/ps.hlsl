#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
	float4 col1 = t1.Sample(s0,input.texCoord);
    return col1 + t0.Sample(s0, input.texCoord);
}