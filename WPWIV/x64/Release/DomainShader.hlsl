#include "GlobalInclude.hlsli"

Texture2D flowmap : register(t1);
Texture2D verticalFilter1 : register(t2);
Texture2D obstacle : register(t7);

SamplerState wrapSampler : register(s0);
SamplerState clampSampler : register(s1);

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation, // float2 for quad
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS_OUTPUT> patch)
{
    DS_OUTPUT Output;
    
    float3 pos = BLERP3(patch[0].pos, patch[1].pos, patch[3].pos, patch[2].pos, domain);
	
    float2 texCoord = BLERP2(patch[0].texCoord, patch[1].texCoord, patch[3].texCoord, patch[2].texCoord, domain);
    //float3 nor = float3(0, 1, 0); //PER VERTEX NORMAL

    if(mode==0||mode==10||mode==11)//0 - default, 10 - normal
    {
        float ob = obstacle.SampleLevel(clampSampler, texCoord, 0).x;
        if (ob > obstacleThresholdWave)
        {
            //do nothing
        }
        else
        {
            float4 deviation = Flow(texCoord, time * timeScale * flowSpeed, flowmap, wrapSampler, verticalFilter1, wrapSampler);
            //float4 deviation = FlowHeightWithNormal(texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2, nor); //PER VERTEX NORMAL
            pos.y += deviation.y;
            pos.x += deviation.x;
            pos.z += deviation.z;
        }
    }
	
    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;
	Output.PosW = pos;
    //Output.nor = nor; //PER VERTEX NORMAL

    return Output;
}
