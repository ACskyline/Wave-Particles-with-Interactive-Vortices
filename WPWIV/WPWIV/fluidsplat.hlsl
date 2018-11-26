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

	if (Curimpulse == 0 && temperaturestate.x == 0)
	{
		col = t6.Sample(s0, coord);
	}
	if (Curimpulse == 0 && temperaturestate.x == 1)
	{
		col = t7.Sample(s0, coord);
	}
	if (Curimpulse == 1 && densitystate.x == 0)
	{
		col = t10.Sample(s0, coord);
	}
	if (Curimpulse == 1 && densitystate.x == 1)
	{
		col = t11.Sample(s0, coord);
	}

	if (Curimpulse == 2 && velocitystate.x == 0)
	{
		col = t4.Sample(s0, coord);
	}
	if (Curimpulse == 2 && velocitystate.x == 1)
	{
		col = t5.Sample(s0, coord);
	}
 
	float4 centpt = float4(0.3, 0.5, 0, 1);
	float d = distance(centpt, coord);
	if (Curimpulse == 0)//temp
	{
		if (d < 0.1)
		{
			col = float4(0.5, 0.5, 0.5, 1);
		}
	}
	else if (Curimpulse == 1)//density
	{
		if (d < 0.1)
		{
			col = float4(0.5, 0.5, 0.5, 1);
		}
	}

	else if (Curimpulse == 2)//velocity
	{
		if (d < 0.1)
		{
			col = float4(0.8, 0, 0.8, 1);
		}
	
	}
	float4 ob = t2.Sample(s0, coord);
	if(ob.x!=0)
	col += float4(0.002, 0.002, 0.002, 0);
	return col;

}