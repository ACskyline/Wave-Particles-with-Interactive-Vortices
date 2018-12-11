#include "GlobalInclude.hlsli"

Texture2D obstacleTex : register(t0);
Texture2D velocityTex : register(t1);

SamplerState wrapSampler : register(s0);

float vorticity(float2 T)
{
    float2 vN = velocityTex.Sample(wrapSampler, T + float2(0, 1.0 / textureHeightFluid)).xy;
    float2 vS = velocityTex.Sample(wrapSampler, T + float2(0, -1.0 / textureHeightFluid)).xy;
    float2 vE = velocityTex.Sample(wrapSampler, T + float2(1.0 / textureWidthFluid, 0)).xy;
    float2 vW = velocityTex.Sample(wrapSampler, T + float2(-1.0 / textureWidthFluid, 0)).xy;
    return 0.5 / fluidCellSize * ((vE.y - vW.y) - (vN.x - vS.x));
}

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
        col = velocityTex.Sample(wrapSampler, input.texCoord);

        float vorC = vorticity(input.texCoord);

        float vorN = vorticity(float2(0, 1.0 / textureHeightFluid) + input.texCoord);
        float vorS = vorticity(float2(0, -1.0 / textureHeightFluid) + input.texCoord);
        float vorE = vorticity(float2(1.0 / textureHeightFluid, 0) + input.texCoord);
        float vorW = vorticity(float2(-1.0 / textureHeightFluid, 0) + input.texCoord);

        float2 force = 0.5 / fluidCellSize * float2(abs(vorN) - abs(vorS), abs(vorE) - abs(vorW));

        //safe normalize
        float epsilon = EPSILON;
        force = force * rsqrt(max(dot(force, force), epsilon));

        force *= fluidCellSize * vorC * float2(1, -1);

        float2 splatDir = float2(splatDirU, splatDirV);
        col += float4(splatDir * splatScale + force * timeStepFluid * vorticityScale, 0, 0);
    }

    return col;
}