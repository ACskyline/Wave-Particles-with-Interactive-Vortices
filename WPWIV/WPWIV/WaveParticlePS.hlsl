#include "GlobalInclude.hlsli"

float4 main(WAVE_PARTICLE input) : SV_TARGET
{
    // return interpolated color
    return float4(input.height, 0, 0, 1);
}