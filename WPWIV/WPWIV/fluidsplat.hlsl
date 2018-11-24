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
 
	float4 centpt = float4(0.5, 0.5, 0, 1);
	float d = distance(centpt, coord);
	if (Curimpulse == 0)//temp
	{
		if (d < 0.1)
		{
			col = float4(0.3, 0.3, 0.3, 1);
		}
	}
	else if (Curimpulse == 1)//density
	{
		if (d < 0.1)
		{
			col = float4(0.3, 0.3, 0.3, 1);
		}
	}
	return col;

}