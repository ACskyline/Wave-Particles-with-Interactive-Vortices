#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float4 col = float4(0,0,0,1);
	float2 coord = input.uv;
	float4 centpt = float4(0.5, 0.5, 0, 1);
	float d = distance(centpt, coord);	
	col = float4(0.2, 0.2, 0.2, 1);
	if (d <0.2)
	{
		col = float4(0.8,0.8, 0.8, 1);
	}

	return  col;
}