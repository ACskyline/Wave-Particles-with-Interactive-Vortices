#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D densityTex : register(t1);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 col = float4(0, 0, 0, 1);
 
    float4 ob = obstacleTex.Sample(wrapSampler, input.texCoord);

    if (ob.x > obstacleThresholdFluid)
    {
        //do nothing
    }
    else
    {
        float2 T = input.texCoord;
        col = densityTex.Sample(wrapSampler, T);
        if (length(T - float2(splatDensityU, splatDensityV)) < splatDensityRadius )
        {
            col += float4(splatDensityScale, 0, 0, 0);
        }
    }

    return col;
}