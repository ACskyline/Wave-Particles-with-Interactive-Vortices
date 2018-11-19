#include "GlobalInclude.hlsli"

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

    float3 pos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
    float2 texCoord = patch[0].texCoord * domain.x + patch[1].texCoord * domain.y + patch[2].texCoord * domain.z;
    
    float4 texColor = t1.SampleLevel(s1, texCoord, 0);
    pos.y += texColor.r * waveParticleScale;

    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;

	return Output;
}
