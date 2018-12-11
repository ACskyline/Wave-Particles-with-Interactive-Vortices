#include "GlobalInclude.hlsli"

VS_CONTROL_POINT_OUTPUT main(VS_INPUT input)
{
    VS_CONTROL_POINT_OUTPUT output;
    output.pos = input.pos;
    output.texCoord = input.texCoord;
    return output;
}