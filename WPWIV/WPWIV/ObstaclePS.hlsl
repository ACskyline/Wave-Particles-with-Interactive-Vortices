#include "GlobalInclude.hlsli"

Texture2D obstacle : register(t0);

SamplerState wrapSampler : register(s0);
SamplerState clampSampler : register(s1);

float4 main(DS_OUTPUT_2 input) : SV_TARGET
{
    float3 lightDir = normalize(float3(1, 1, 1));
    float3 color = float3(0.80, 0.52, 0.28);
    float height = clamp(input.wpos.y, 0, 1); //no need to clamp bc we are using DXGI_FORMAT_R8G8B8A8_UNORM
    float halfLambert = dot(lightDir, normalize(input.nor)) * 0.5 + 0.5;
    return float4(height * color * halfLambert, 1);
}
