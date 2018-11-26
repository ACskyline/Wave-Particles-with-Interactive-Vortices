#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(VSQuadOut input) : SV_TARGET
{
	float2 TC = input.uv;
	float T = 0;
	float D = 0;
	float4 V = float4(0, 0,0,1);
	if (velocitystate.x == 0)
	{
		V = t4.Sample(s0, TC);
	}
	else if (velocitystate.x == 1)
	{
		V = t5.Sample(s0, TC);
	}

	float4 col = V;


	if (temperaturestate.x == 0)
	{
		float T = t6.Sample(s0, TC).x;
	}
	else if (temperaturestate.x == 1)
	{
		float T = t7.Sample(s0, TC).x;
	}

	if (T > AmbientTemperature)
	{
		if (densitystate.x == 0)
		{
			D = t10.Sample(s0, TC).x;
		}
		else if (densitystate.x == 1)
		{
			D = t11.Sample(s0, TC).x;
		}
		col += (TimeStep*(T - AmbientTemperature)*SmokeBuoyancy - D * SmokeWeight*0.1)*float4(0, 1, 0, 0);

	}

	return col;

}