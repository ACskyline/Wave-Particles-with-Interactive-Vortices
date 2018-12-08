#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 col = float4(0, 0, 0, 1);
 
    float4 ob = obstacleTex.Sample(wrapSampler, input.texCoord);

    if (ob.x > obstacleThresholdFluid)
    {
        //do nothing
    }
    else if(input.texCoord.x<0)
    {
        float2 splatDir = float2(splatDirU, splatDirV);
        col += float4(splatDir * splatScale, 0, 0);
    }

    return col;
}