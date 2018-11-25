#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float4 col = float4(0, 0, 0, 1);
	float2 T = input.uv;
	float3 oC = t2.Sample(s0, T).xyz;
	if (oC.x  == 0)
	{
		col = float4(0, 0, 0, 1);
	}

	//find neighboring pressure
	float pN = 0;
	float pS = 0;
	float pE = 0;
	float pW = 0;
	float pC = 0;
	if (pressurestate.x == 0)
	{
		pN = t8.Sample(s0, T + float2(0, 1.0 / size.y)).x;
		pS = t8.Sample(s0, T + float2(0, -1.0 / size.y)).x;
		pE = t8.Sample(s0, T + float2(1.0 / size.x, 0)).x;
		pW = t8.Sample(s0, T + float2(-1.0 / size.x, 0)).x;
		pC = t8.Sample(s0, T).x;
	}
	if (pressurestate.x == 1)
	{
		pN = t9.Sample(s0, T + float2(0, 1.0 / size.y)).x;
		pS = t9.Sample(s0, T + float2(0, -1.0 / size.y)).x;
		pE = t9.Sample(s0, T + float2(1.0 / size.x, 0)).x;
		pW = t9.Sample(s0, T + float2(-1.0 / size.x, 0)).x;
		pC = t9.Sample(s0, T).x;
	}

	//find neighboring obstacles

	float3 oN = t2.Sample(s0, T + float2(0, 1.0 / size.y)).xyz;
	float3 oS = t2.Sample(s0, T + float2(0, -1.0 / size.y)).xyz;
	float3 oE = t2.Sample(s0, T + float2(1.0 / size.x, 0)).xyz;
	float3 oW = t2.Sample(s0, T + float2(-1.0 / size.x, 0)).xyz;


	//use center pressure for solid cells
	float2 obstV = float2(0, 0);
	float2 vMask = float2(1, 1);

	if (oN.x == 0) { pN = pC; obstV.y = 0; vMask.y = 0; }
	if (oS.x == 0) { pS = pC; obstV.y = 0; vMask.y = 0; }
	if (oE.x == 0) { pE = pC; obstV.x = 0; vMask.x = 0; }
	if (oW.x == 0) { pW = pC; obstV.x = 0; vMask.x = 0; }

	//enforce the free slip boundary condition
	float2 oldV = float2(0, 0);

	if (velocitystate.x == 0)
	{
		oldV = t4.Sample(s0, T).xy;
	}
	else if (velocitystate.x == 1)
	{
		oldV = t5.Sample(s0, T).xy;
	}
	
	float2 grad = float2(pE - pW, pN - pS)*GradientScale*0.08;
	float2 newV = oldV - grad;

	col = float4((vMask*newV) + obstV,0,1);
	// return interpolated color
	return col;

}