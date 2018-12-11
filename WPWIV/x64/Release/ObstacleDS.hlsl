#include "GlobalInclude.hlsli"

Texture2D obstacle : register(t0);

SamplerState wrapSampler : register(s0);
SamplerState clampSampler : register(s1);

[domain("quad")]
DS_OUTPUT_2 main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation, // float2 for quad
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_OUTPUT> patch)
{
    DS_OUTPUT_2 Output;
    
    float3 pos = BLERP3(patch[0].pos, patch[1].pos, patch[3].pos, patch[2].pos, domain);
    float2 texCoord = BLERP2(patch[0].texCoord, patch[1].texCoord, patch[3].texCoord, patch[2].texCoord, domain);
    
    float ob = clamp(obstacle.SampleLevel(wrapSampler, texCoord, 0).x, 0, 1);
    pos.y += ob * obstacleScale;
    
    Output.nor = ObstacleMapToNormal(texCoord, obstacle, wrapSampler);
    Output.wpos = pos;
    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;

    return Output;
}
