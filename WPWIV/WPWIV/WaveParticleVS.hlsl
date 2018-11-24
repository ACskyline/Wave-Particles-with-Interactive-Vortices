#include "GlobalInclude.hlsli"

WAVE_PARTICLE main(VS_INPUT input)
{
    WAVE_PARTICLE output;
    output.pos = float4(input.pos.xy, 0.5, 1);
    output.direction = input.texCoord;
    output.height = input.pos.z;
    output.speed = input.pos.z;
    return output;
}