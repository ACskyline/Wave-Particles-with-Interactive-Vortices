#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D pressureTex : register(t1);
Texture2D velocityTex : register(t2);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 col = float4(0, 0, 0, 1);
    float2 T = input.texCoord;
    float oC = obstacleTex.Sample(wrapSampler, T).x;
    if (oC.x > obstacleThresholdFluid)
    {
        col = float4(0, 0, 0, 1);
    }
    else
    {
	    //find neighboring pressure
        float pN = pressureTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).x;
        float pS = pressureTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).x;
        float pE = pressureTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).x;
        float pW = pressureTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).x;
        float pC = pressureTex.Sample(wrapSampler, T).x;

	    //find neighboring obstacles
        float3 oN = obstacleTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).xyz;
        float3 oS = obstacleTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).xyz;
        float3 oE = obstacleTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).xyz;
        float3 oW = obstacleTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).xyz;

	    //use center pressure for solid cells
        float2 obstV = float2(0, 0);
        float2 vMask = float2(1, 1);

	    //enforce the free slip boundary condition
        float2 oldV = velocityTex.Sample(wrapSampler, T).xy;

        if (oN.x > obstacleThresholdFluid)
        {
            pN = pC;
            obstV.y = 0;
            vMask.y = 0;
        }
        if (oS.x > obstacleThresholdFluid)
        {
            pS = pC;
            obstV.y = 0;
            vMask.y = 0;
        }
        if (oE.x > obstacleThresholdFluid)
        {
            pE = pC;
            obstV.x = 0;
            vMask.x = 0;
        }
        if (oW.x > obstacleThresholdFluid)
        {
            pW = pC;
            obstV.x = 0;
            vMask.x = 0;
        }


        float halfInvCellSize = 0.5 / fluidCellSize;

        float2 grad = float2(pE - pW, pN - pS) * halfInvCellSize;
        float2 newV = oldV - grad;

	    // return interpolated color
        col = float4((vMask * newV) + obstV, 0, 1);
    }

    return col;
}
