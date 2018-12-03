#include "GlobalInclude.hlsli"

const float VolumeTop = 5;
const float VolumeBottom = 0;

const float3 IsotropicLightTop = float3(0, 0.6, 0.8);
const float3 IsotropicLightBottom = float3(0, 0, 0.5);

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
        return t0.Sample(s1, input.texCoord);
    }
    else if (mode == 1)//1 - flow map, 
    {
        return t1.Sample(s1, input.texCoord);
    }
    else if (mode == 2)//2 - flow map driven texture, 
    {
        return Flow(input.texCoord, time * timeScale * flowSpeed, t1, s2, t0, s2);
    }
    else if (mode==3||mode==4||mode==5||mode==6)//3, 4, 5, 6 - blur
    {
        return t2.Sample(s1, input.texCoord);
    }
    else if (mode == 7)//7 - normal
    {
        //float3 normal = input.nor; //PER VERTEX NORMAL

        //PER PIXEL NORMAL
        //MODEL SPACE
        float3 normal = FlowHeightForNormal(input.texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2);
        //WORLD SPACE, THIS IS IMPORTANT TO KEEP NORMAL CORRECT WITH LARGE SCALE WATER SURFACE
        //RIGHT MULTIPLY THE TRANSPOSE OF AN INVERSE MATRIX IS LEFT MULTIPLY THE INVERSE MATRIX
        //ADDITIONALY MATRIX PASSED FROM CPU TO GPU WILL TRANSPOSE AGAIN, SO IT CANCELED OUT
        normal = normalize(mul(float4(normal, 0), modelInv));
        return float4((normal + 1.0) * 0.5, 1.0);//[-1, 1] to [0, 1] for display purpose
    }
	else if (mode == 8)
	{
		float3 normal = FlowHeightForNormal(input.texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2);
		normal = normalize(mul(float4(normal, 0), modelInv));
		
		//specular part
		float3 lightdir = normalize(float3(2,lighthight,10));
		float3 worldpos = float3(input.PosW);
		float3 lightpos = float3(2, lighthight, 10);
		float3 ld = normalize(lightpos - worldpos);
		float3 vp = float3(vx, vy, vz);
		vp = mul(float4(vp, 0), model);

		float3 vd =normalize(vp - worldpos);
		float3 halfwaydir = normalize(vd + ld);
		float spec = pow(max((dot(normal, halfwaydir)), 0.0), shiness);
		//specular end

		// water color part
		float depth = worldpos.y;
		float ExtinctionCoeff = extinctcoeff;
		float3 surfacenor = float3(0, 1, 0);
		float3 watercol = float3(0, 0.5, 1);
		float3 suncol = float3(1, 1, 1);
		float3 groundcol = float3(0.4, 0.3, 0.2);
		float3 topplanecol = watercol * dot(surfacenor, ld)*suncol;
		float3 bottomplanecol = groundcol * dot(surfacenor, ld) * exp(-depth* ExtinctionCoeff);
		float FoamTurbulance = 0;//for adding extra foam features(sample from foam texture)
		float top = 3;
		float bottom = 0;
		float3 bottompos = float3(worldpos.x,0,worldpos.z);//right now no vriance in depth of river bed, in the future will need to sample from blockers(changing river bed)
		float3 halfpoint = lerp(bottompos, worldpos, 0.5f + FoamTurbulance);
		float3 ambientcol = ComputeAmbientColor(halfpoint, ExtinctionCoeff, bottomplanecol, topplanecol, top, bottom);

		//water color end

		float3 refracterm = float3(0, 0, 0);

		//if (depth > 1.2)
		//{
		//	refracterm = lerp(watercol, suncol, depth - 1.2)* dot(surfacenor, ld);
		//}
		
		//lambertterm for light occulusion

		float lo = dot(normal, ld);

		float lambert = dot(normal, vd);
		float4 lambeterm = float4(0.1,0.1,0.1,0)*(lambert);
		float4 loterm = float4(0.1, 0.1, 0.1, 0)*(1-lo);
		//spec *= dot(surfacenor, ld);


		return (float4(ambientcol,1) + float4(spec, spec, spec, 1)+ float4(refracterm,1)) + lambeterm;//[-1, 1] to [0, 1] for display purpose

	}

    return float4(0, 0, 0, 1);
}
