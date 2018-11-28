#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float4 divergence = t12.Sample(s0,input.uv);
	divergence = float4(divergence.x, 0, 0, 1);
	float4 coldens = t10.Sample(s0,input.uv);
	coldens = float4(coldens.x, coldens.x, coldens.x, 1);
	float4 vel = t4.Sample(s0, input.uv);
	float4 pressure = t8.Sample(s0, input.uv);
	coldens *= 2;
	clamp(coldens, 0, 0.8);
	return coldens;
}