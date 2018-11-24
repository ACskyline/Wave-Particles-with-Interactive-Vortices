#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(DS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    return t1.Sample(s0, input.texCoord);

}