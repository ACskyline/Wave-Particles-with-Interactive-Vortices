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
    
    if(mode==0||mode==7)
    {
        float4 result = float4(0,0,0,1);
        if(mode==0)
        {
            float4 tx1, tx2;
            float timeInt = float(time * timeScale * flowSpeed) / (1 * 2);//interval is always 1
            float2 fTime = frac(float2(timeInt, timeInt + .5));
            float4 flowMap = t2.SampleLevel(s1, texCoord, 0);
            float2 flowDir = (flowMap.xy - float2(0.5, 0.5)) * 2;
            float2 flowUV1 = texCoord - (flowDir / 2) + fTime.x * flowDir.xy;
            float2 flowUV2 = texCoord - (flowDir / 2) + fTime.y * flowDir.xy;
            tx1 = t0.SampleLevel(s2, flowUV1, 0);
            tx2 = t0.SampleLevel(s2, flowUV2, 0);
            result = lerp(tx1, tx2, abs(2 * frac(timeInt) - 1));
        }
        else if(mode==7)
        {
            result = t0.SampleLevel(s0, texCoord, 0);
        }
        pos.y += result.y * heightScale;
        pos.x -= result.x * dxScale;
        pos.z -= result.z * dzScale;
    }
    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;

    return Output;
}
