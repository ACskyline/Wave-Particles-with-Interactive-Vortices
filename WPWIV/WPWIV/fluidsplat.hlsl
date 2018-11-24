#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float2 coord = input.uv;
	float4 col = float4(0, 0, 0, 1);
	//if (coord.y > 0.8)
	//{
	//	col = float4(1, 1, 1, 1);
	//}
	// return interpolated color
	return col;

}