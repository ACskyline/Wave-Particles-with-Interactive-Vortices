#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	// return interpolated color
	float4 col = float4(0,0,0,0);
	float2 T = input.uv;
	float4 pN = float4(0, 0, 0, 0);
	float4 pS = float4(0, 0, 0, 0);
	float4 pE = float4(0, 0, 0, 0);
	float4 pW = float4(0, 0, 0, 0);
	float4 pC = float4(0, 0, 0, 0);

	if (pressurestate.x == 0)
	{
		pN = t8.Sample(s0, T + float2(0, 1.0 / size.y));
		pS = t8.Sample(s0, T + float2(0, -1.0 / size.y));
		pE = t8.Sample(s0, T + float2(1.0 / size.x, 0));
		pW = t8.Sample(s0, T + float2(-1.0 / size.x, 0));
		pC = t8.Sample(s0, T);
	}
	if (pressurestate.x == 1)
	{
		pN = t9.Sample(s0, T + float2(0, 1.0 / size.y));
		pS = t9.Sample(s0, T + float2(0, -1.0 / size.y));
		pE = t9.Sample(s0, T + float2(1.0 / size.x, 0));
		pW = t9.Sample(s0, T + float2(-1.0 / size.x, 0));
		pC = t9.Sample(s0, T);
	}

	//neighbor obstacle pixel

	float3 oN = t2.Sample(s0, T + float2(0, 1.0 / size.y)).xyz;
	float3 oS = t2.Sample(s0, T + float2(0, -1.0 / size.y)).xyz;
	float3 oE = t2.Sample(s0, T + float2(1.0 / size.x, 0)).xyz;
	float3 oW = t2.Sample(s0, T + float2(-1.0 / size.x, 0)).xyz;

	//use center pressure for solid cells
	if (oN.x == 0) pN = float4(1,1,1,1);
	if (oS.x == 0) pS = float4(1, 1, 1, 1);
	if (oE.x == 0) pE = float4(1, 1, 1, 1);
	if (oW.x == 0) pW = float4(1, 1, 1, 1);

	float4 bC = t12.Sample(s0, T);

	float alpha = -cellsize * cellsize;
	float invbeta = 0.243;//DO NOT change, very delicate

	col = (pW + pE + pS + pN + alpha * bC)*invbeta;

	return col;

}