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
	float2 invsize = float2(1 / size.x,1 / size.y);
	float solid = t2.Sample(s0, coord).x;
	float scale = 1;
	if (solid == 0)
	{
		col = float4(0, 0, 0, 1);
		return col;
	}
	else
	{

			if (velocitystate.x == 0 && densitystate.x == 0)
			{
				float2 u = t4.Sample(s0, coord).xy;
				float2 c = coord - TimeStep * u*scale;
				col = DensityDissipation * t10.Sample(s0, c);
			}

			if (velocitystate.x == 1 && densitystate.x == 0)
			{
				float2 u = t5.Sample(s0, coord).xy;
				float2 c = coord - TimeStep * u*scale;
				col = DensityDissipation * t10.Sample(s0, c);
			}
			 if (velocitystate.x == 0 && densitystate.x == 1)
			{
				float2 u = t4.Sample(s0, coord).xy;
				float2 c = coord - TimeStep * u*scale;
				col = DensityDissipation * t11.Sample(s0, c);
			}
			 if (velocitystate.x == 1 && densitystate.x == 1)
			{
				float2 u = t5.Sample(s0, coord).xy;
				float2 c = coord - TimeStep * u*scale;
				col = DensityDissipation * t11.Sample(s0, c);
			}
		 return col;
		

	}
	return col /*+float4(0.1,0.1,0.1,1)*/;//write to pong of velocity buffers

}