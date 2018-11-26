#include "GlobalInclude.hlsli"

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation, // float2 for quad
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_OUTPUT> patch)
{
	DS_OUTPUT Output;
    
    float3 pos = BLERP3(patch[0].pos, patch[1].pos, patch[3].pos, patch[2].pos, domain);
    float2 texCoord = BLERP2(patch[0].texCoord, patch[1].texCoord, patch[3].texCoord, patch[2].texCoord, domain);
    
    float4 data = t0.SampleLevel(s0, texCoord, 0);
    //pos.xz += data.gb;
    //pos.y += data.r * heightScale;
    //pos.x += data.b * dxScale;
    //pos.z += data.a * dzScale;
    pos.y += data.z * heightScale;
    pos.x += data.x * dxScale;
    pos.z += data.y * dzScale;

    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;

	return Output;
}
