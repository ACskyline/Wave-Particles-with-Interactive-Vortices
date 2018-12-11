#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D pressureTex : register(t1);
Texture2D divergenceTex : register(t2);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	// return interpolated color
    float4 col = float4(0, 0, 0, 0);
    float2 T = input.texCoord;
    float4 pN = float4(0, 0, 0, 0);
    float4 pS = float4(0, 0, 0, 0);
    float4 pE = float4(0, 0, 0, 0);
    float4 pW = float4(0, 0, 0, 0);
    float4 pC = float4(0, 0, 0, 0);
    
    pN = pressureTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid));
    pS = pressureTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid));
    pE = pressureTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0));
    pW = pressureTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0));
    pC = pressureTex.Sample(wrapSampler, T);

	//neighbor obstacle pixel

    float oN = obstacleTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).x;
    float oS = obstacleTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).x;
    float oE = obstacleTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).x;
    float oW = obstacleTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).x;
    
	//use center pressure for solid cells
    if (oN > obstacleThresholdFluid)
        pN = pC;// * jacobiObstacleScale;
    if (oS > obstacleThresholdFluid)
        pS = pC;// * jacobiObstacleScale;
    if (oE > obstacleThresholdFluid)
        pE = pC;// * jacobiObstacleScale;
    if (oW > obstacleThresholdFluid)
        pW = pC;// * jacobiObstacleScale;

    float4 bC = divergenceTex.Sample(wrapSampler, T);

    float alpha = -fluidCellSize * fluidCellSize;

    col = 1.01*(pW + pE + pS + pN + alpha * bC) * 0.25;//    jacobiInvBeta;

    return col;
}