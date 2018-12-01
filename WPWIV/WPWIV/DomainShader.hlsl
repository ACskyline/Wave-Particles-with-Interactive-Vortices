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
    //float3 nor = float3(0, 1, 0); //PER VERTEX NORMAL

    if(mode==0||mode==7)//0 - default, 7 - normal
    {
        float4 deviation = Flow(texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2);
        //float4 deviation = FlowHeightWithNormal(texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2, nor); //PER VERTEX NORMAL
        pos.y += deviation.y;
        pos.x += deviation.x;
        pos.z += deviation.z;
    }
    Output.pos = mul(mul(viewProj, model), float4(pos, 1));
    Output.texCoord = texCoord;
    //Output.nor = nor; //PER VERTEX NORMAL

    return Output;
}
