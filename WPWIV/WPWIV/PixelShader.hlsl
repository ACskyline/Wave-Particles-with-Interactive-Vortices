#include "GlobalInclude.hlsli"

//float4 main(VS_OUTPUT input) : SV_TARGET
//{
//    // return interpolated color
//    return t1.Sample(s0, input.texCoord);
//}

float4 main(DS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    //return float4(t0.Sample(s0, input.texCoord).x, 0, 0, 1);
    return t1.Sample(s0, input.texCoord);
    //float col = t0.Sample(s0, input.texCoord).r;
    //return float4(col, col, col, 1);
}