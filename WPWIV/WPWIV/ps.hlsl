#include "GlobalInclude.hlsli"

float4 main(VSQuadOut input) : SV_TARGET
{
    // return interpolated color
	//float4 col1 = t1.Sample(s0,input.texCoord);
    return  t1.Sample(s0, input.uv);
}