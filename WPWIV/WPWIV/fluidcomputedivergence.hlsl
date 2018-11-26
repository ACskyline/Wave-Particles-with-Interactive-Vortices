#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);

float4 main(VSQuadOut input) : SV_TARGET
{
	float2 T = input.uv;

	float2 vN = float2(0, 0);
	float2 vS = float2(0, 0);
	float2 vE = float2(0, 0);
	float2 vW = float2(0, 0);

	if (velocitystate.x == 0)
	{
		vN = t4.Sample(s0, T + float2(0, 1 / size.y)).xy;
		vS = t4.Sample(s0, T + float2(0, -1 / size.y)).xy;
		vE = t4.Sample(s0, T + float2(1/size.x ,0)).xy;
		vW = t4.Sample(s0, T + float2(-1/size.x, 0)).xy;

	}
	else if (velocitystate.x == 1)
	{
		vN = t5.Sample(s0, T + float2(0, 1 / size.y)).xy;
		vS = t5.Sample(s0, T + float2(0, -1 / size.y)).xy;
		vE = t5.Sample(s0, T + float2(1 / size.x, 0)).xy;
		vW = t5.Sample(s0, T + float2(-1 / size.x, 0)).xy;
	}

	//find neighbor obstacles:

	float3 oN = t2.Sample(s0, T + float2(0, 1.0 / size.y)).xyz;
	float3 oS = t2.Sample(s0, T + float2(0, -1.0 / size.y)).xyz;
	float3 oE = t2.Sample(s0, T + float2(1.0 / size.x, 0)).xyz;
	float3 oW = t2.Sample(s0, T + float2(-1.0 / size.x, 0)).xyz;

	if (oN.x == 0) vN = float2(0, 0);
	if (oS.x == 0) vS = float2(0, 0);
	if (oE.x == 0) vE = float2(0, 0);
	if (oW.x == 0) vW = float2(0, 0);

	float halfinvcellsize = 10 / 1.25;//DO NOT change, very delicate

	float4 col = float4(0, 0, 0, 1);
	col = halfinvcellsize * (vE.x - vW.x + vN.y - vS.y);

	// return interpolated color
	return col;

}