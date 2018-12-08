#include "GlobalInclude.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float2 brushOffset = float2(brushOffsetU, brushOffsetV);
    output.pos = float4(input.pos.xy * brushScale + brushOffset, 0.5, 1);
    output.texCoord = input.texCoord;
    return output;
}
