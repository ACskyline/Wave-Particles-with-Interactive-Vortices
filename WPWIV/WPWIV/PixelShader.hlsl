#include "GlobalInclude.hlsli"

float4 main(DS_OUTPUT input) : SV_TARGET
{
    if (mode == 0 || mode == 7)//0 - default, 7 - wave particle driven deviation
    {
        return t1.Sample(s1, input.texCoord);
    }
    else if (mode == 1)//1 - flow map, 
    {
        return t2.Sample(s1, input.texCoord);
    }
    else if (mode == 2)//2 - flow map driven texture, 
    {
        float3 result, tx1, tx2;
        float timeInt = float(time * timeScale * 0.0001) / (1 * 2);
        float2 fTime = frac(float2(timeInt, timeInt + .5));
        float4 flowMap = t2.Sample(s1, input.texCoord);
        float2 flowDir = (flowMap.xy - float2(0.5, 0.5)) * 2;
        float2 flowUV1 = input.texCoord - (flowDir / 2) + fTime.x * flowDir.xy;
        float2 flowUV2 = input.texCoord - (flowDir / 2) + fTime.y * flowDir.xy;
        tx1 = t1.Sample(s1, flowUV1).xyz;
        tx2 = t1.Sample(s1, flowUV2).xyz;
        result = lerp(tx1, tx2, abs(2 * frac(timeInt) - 1));
        return float4(result, 1);
    }
    else if (mode==3||mode==4||mode==5||mode==6)
    {
        return t0.Sample(s1, input.texCoord);
    }
    return float4(0,0,0,1);
}