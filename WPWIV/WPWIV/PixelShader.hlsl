#include "GlobalInclude.hlsli"

Texture2D albedo : register(t0);
Texture2D flowmap : register(t1);
Texture2D verticalFilter1 : register(t2);
Texture2D verticalFilter2 : register(t3);
Texture2D density : register(t4);
Texture2D pressure : register(t5);
Texture2D divergence : register(t6);

SamplerState wrapSampler : register(s0);
SamplerState clampSampler : register(s1);

float Ei(float z)
{
	const float EulerMascheroniConstant = 0.577216f;
	const float z2 = z * z;
	const float z3 = z2 * z;
	const float z4 = z3 * z;
	const float z5 = z4 * z;
	return EulerMascheroniConstant + log(z) + z + z2 / 4.f + z3 / 18.f + z4 / 96.f + z5 / 600.f;
}

float3 ComputeAmbientColor(float3 Position, float ExtinctionCoeff, float3 IsotropicLightBottom, float3 IsotropicLightTop, float VolumeTop, float VolumeBottom) {
	float Hp = VolumeTop - Position.y; // Height to the top of the volume
	float a = -ExtinctionCoeff * Hp;
	float3 IsotropicScatteringTop = IsotropicLightTop * max(0.0, exp(a) - a * Ei(a));
	float Hb = Position.y - VolumeBottom; // Height to the bottom of the volume
	a = -ExtinctionCoeff * Hb;
	float3 IsotropicScatteringBottom = IsotropicLightBottom * max(0.0, exp(a) - a * Ei(a));
	return IsotropicScatteringTop + IsotropicScatteringBottom;
}

float4 main(DS_OUTPUT input) : SV_TARGET
{
    if (mode == 0)//0 - default,
    {
        float d = abs(divergence.Sample(clampSampler, input.texCoord).x);
        float3 water = float3(0, 0, 0.5);
        float3 foam = float3(0, 0, 0);
        if (d > 0)
        {
            foam = Flow(input.texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, albedo, wrapSampler).xyz;
        }
        return float4(d * foam * foamScale + (1 - d) * water, 1);
    }
    else if (mode == 1)//1 - flow map, 
    {
        float2 flow = flowmap.Sample(clampSampler, input.texCoord).xy;
        return float4(flow, 0, 1);
    }
    else if (mode == 2)//2 - density, 
    {
        float d = abs(density.Sample(clampSampler, input.texCoord).x);
        return float4(d, d, d, 1);
    }
    else if (mode == 3)//3 - divergence, 
    {
        float d = abs(divergence.Sample(clampSampler, input.texCoord).x);
        return float4(d, d, d, 1);
    }
    else if (mode == 4)//4 - pressure, 
    {
        return pressure.Sample(clampSampler, input.texCoord);
    }
    else if (mode == 5)//5 - flow map driven texture, 
    {
        return float4(Flow(input.texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, albedo, wrapSampler).xyz, 1);
    }
    else if (mode==6||mode==7||mode==8||mode==9)//6, 7, 8, 9 - blur
    {
        return verticalFilter1.Sample(clampSampler, input.texCoord);
    }
    else if (mode == 10)//7 - normal
    {
        //float3 normal = input.nor; //PER VERTEX NORMAL

        //PER PIXEL NORMAL
        //MODEL SPACE
        float3 normal = FlowHeightForNormal(input.texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, verticalFilter1, wrapSampler);
        //WORLD SPACE, THIS IS IMPORTANT TO KEEP NORMAL CORRECT WITH LARGE SCALE WATER SURFACE
        //RIGHT MULTIPLY THE TRANSPOSE OF AN INVERSE MATRIX IS LEFT MULTIPLY THE INVERSE MATRIX
        //ADDITIONALY MATRIX PASSED FROM CPU TO GPU WILL TRANSPOSE AGAIN, SO IT CANCELED OUT
        normal = normalize(mul(float4(normal, 0), modelInv));
        return float4((normal + 1.0) * 0.5, 1.0);//[-1, 1] to [0, 1] for display purpose
    }
	else if (mode == 11)
	{
		float3 normal = FlowHeightForNormal(input.texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, verticalFilter1, wrapSampler);
		//WORLD SPACE, THIS IS IMPORTANT TO KEEP NORMAL CORRECT WITH LARGE SCALE WATER SURFACE
		//RIGHT MULTIPLY THE TRANSPOSE OF AN INVERSE MATRIX IS LEFT MULTIPLY THE INVERSE MATRIX
		//ADDITIONALY MATRIX PASSED FROM CPU TO GPU WILL TRANSPOSE AGAIN, SO IT CANCELED OUT
		normal = normalize(mul(float4(normal, 0), modelInv));
		//return float4((normal + 1.0) * 0.5, 1.0);//[-1, 1] to [0, 1] for display purpose


		//divergence

		float d = abs(divergence.Sample(clampSampler, input.texCoord).x);
		float3 foam = float3(0, 0, 0);
		if (d > 0)
		{
			foam = Flow(input.texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, albedo, wrapSampler).xyz;
		}

		float3 foamcol = d * foam * foamScale;

		//specular part
		float3 lightdir = normalize(float3(2, lighthight, 10));
		float3 worldpos = float3(input.PosW);
		float3 ViewPoint = float3(vx, vy, vz);
		float3 ViewDirection = normalize(ViewPoint - worldpos);
		float3 lightpos = float3(2, lighthight, 10);
		float3 NaivelightDir = normalize(lightpos - worldpos);

		float3 halfwaydir = normalize(ViewDirection + NaivelightDir);
		float SpecHighlight = pow(max((dot(normal, halfwaydir)), 0.0), shiness);
		float3 speccol = float3(0, 0, 0);
		float3 ReflectViewDir = float3(0, 0, 0);


		//very fake sky box
		if (dot(ViewDirection, normal))
		{
			float3 skycol1 = float3(0.1, 0.2, 0.3);
			float3 skycol2 = float3(0.2, 0.2, 0.2);
			skycol1 *= 2;
			skycol2 *= 7;
			float3 ReflectViewDir = reflect(ViewDirection, normal);
			ReflectViewDir = normalize(ReflectViewDir);
			speccol = lerp(skycol1, skycol2, abs(ReflectViewDir.y));
			speccol *= 1.5;		
			float3 sundir = normalize(float3(-1, 2, -1));
			float angle = acos(dot(sundir, -ReflectViewDir));
			if (angle < 0.1)
			{
				speccol = lerp(speccol, float3(1, 0.3, 0), (0.1 - angle) / 0.1);
			}
		}
		//specular end

		// water color part


		//float ior = 1.2;
		//float theta = (180.0f / 3.1415926)*acos(dot(ReflectViewDir, normal));
		//if (theta >= 90.0) ior = 1 / ior;
		//else
		//	ior = ior;
		//float R0 = (1 - ior) / (1 + ior)*(1 - ior) / (1 + ior);
		//float RSc = R0 + (1 - R0)*pow(1.0 - abs(dot(normal, ReflectViewDir)), 5);

		//emperical fresnel method
		float R = max(0, min(1, fbias + fscale * pow(1.0 + dot(ViewDirection, normal), fpow)));


		//use jpg method
		float depth = worldpos.y;
		float ExtinctionCoeff = extinctcoeff;
		float3 surfacenor = float3(0, 1, 0);
		float3 watercol = float3(0, 0.6, 1);
		float3 suncol = float3(1, 1, 1);
		float3 groundcol = float3(0.4, 0.3, 0.2);//float3(0, 0.2, 0.5);
		float3 topplanecol = watercol * dot(surfacenor, NaivelightDir)*suncol;
		float3 bottomplanecol = groundcol * dot(surfacenor, NaivelightDir) * exp(-depth * ExtinctionCoeff);
		float FoamTurbulance = 10*foamcol.y;//for adding extra foam features(sample from foam texture)
		float top = 3;
		float bottom = 0;

		float3 bottompos = float3(worldpos.x, 0, worldpos.z);
		//FoamTurbulance = 10*foamcol;
		float3 halfpoint = lerp(bottompos, worldpos, 0.5f + FoamTurbulance);
		float3 ambientcol = ComputeAmbientColor(halfpoint, ExtinctionCoeff, bottomplanecol, topplanecol, top, bottom);


		//water color end


		float4 Rcol = float4(R, R, R, 1);
		float3 specHighlightcol = SpecHighlight * float3(1, 1, 1);
		float3 final = lerp(speccol, ambientcol, R);

		

		return float4(final, 1) + float4(specHighlightcol, 1) + clamp(foampow * float4(foamcol,1),0,1)*0.3;
	}
    return float4(0, 0, 0, 1);
}
