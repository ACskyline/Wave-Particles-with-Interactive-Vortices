#include "GlobalInclude.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(mul(viewProj, model), float4(input.pos, 1));
    output.texCoord = input.texCoord;
    return output;
}