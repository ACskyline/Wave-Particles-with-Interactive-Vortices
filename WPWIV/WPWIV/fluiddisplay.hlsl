#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float4 col = t10.Sample(s0,input.uv);
	col = float4(col.x, col.x, col.x, 1);
	return col;
}