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

    return float4(0, 0, 0, 1);
}
