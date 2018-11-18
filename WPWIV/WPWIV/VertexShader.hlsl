#include "GlobalInclude.hlsli"

//VS_OUTPUT main(VS_INPUT input)
//{
//    VS_OUTPUT output;
//    output.pos = mul(mul(viewProj, model), float4(input.pos, 1));
//    output.texCoord = input.texCoord;
//    return output;
//}

VS_CONTROL_POINT_OUTPUT main(VS_INPUT input)
{
    VS_CONTROL_POINT_OUTPUT output;
    output.pos = input.pos;
    output.texCoord = input.texCoord;
    return output;
}