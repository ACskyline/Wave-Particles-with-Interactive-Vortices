#include "GlobalInclude.hlsli"

Texture2D sourceTex : register(t0);
SamplerState wrapSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_Target
{
    float weights[3] = { 0.01330373, 0.11098164, 0.22508352 };

    float4 col = weights[0] * sourceTex.Sample(wrapSampler, input.texCoord);

    for (int i = 1; i <= 2; i++)
    {
        float offset = i / float(textureWidth);
        float4 hL = weights[i] * sourceTex.Sample(wrapSampler, input.texCoord + float2(-offset, 0));
        float4 hR = weights[i] * sourceTex.Sample(wrapSampler, input.texCoord + float2(offset, 0));
        col += hL + hR;
    }

    return col;
}
