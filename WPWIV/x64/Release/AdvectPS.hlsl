#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D velocityTex : register(t1);
Texture2D srcTex : register(t2);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{

    float4 col = float4(0, 0, 0, 1);
    float solid = obstacleTex.Sample(wrapSampler, input.texCoord).x;

    if (solid > obstacleThresholdFluid)
    {
        col = float4(0, 0, 0, 1);
    }
    else
    {
        float2 u = velocityTex.Sample(wrapSampler, input.texCoord).xy;
        float2 c = input.texCoord - timeStepFluid * u;
        col = fluidDissipation * srcTex.Sample(wrapSampler, c);
    }

    return col;
}
