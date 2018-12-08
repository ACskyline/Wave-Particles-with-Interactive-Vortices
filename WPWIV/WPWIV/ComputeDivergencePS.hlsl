#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D velocityTex : register(t1);

SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float2 T = input.texCoord;

    float2 vN = velocityTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).xy;
    float2 vS = velocityTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).xy;
    float2 vE = velocityTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).xy;
    float2 vW = velocityTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).xy;
    
	//find neighbor obstacles:
    float oN = obstacleTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).x;
    float oS = obstacleTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).x;
    float oE = obstacleTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).x;
    float oW = obstacleTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).x;

    if (oN.x > obstacleThresholdFluid)
        vN = float2(0, 0);
    if (oS.x > obstacleThresholdFluid)
        vS = float2(0, 0);
    if (oE.x > obstacleThresholdFluid)
        vE = float2(0, 0);
    if (oW.x > obstacleThresholdFluid)
        vW = float2(0, 0);

    float halfInvCellSize = 0.5 / fluidCellSize; //DO NOT change, very delicate

    float4 col = halfInvCellSize * (vE.x - vW.x + vN.y - vS.y);//divergenceScale * 

	// return interpolated color
    return col;
}
