#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{ 
	float4 col = float4(0,0,0,0);
    // return interpolated color
	if (4 == 1)
		{
			col = float4(0,0,0,0);
		}
    col = t1.Sample(s0, input.uv);
	return col;

}