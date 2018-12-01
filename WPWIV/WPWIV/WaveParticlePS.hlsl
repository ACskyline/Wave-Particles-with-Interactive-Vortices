#include "GlobalInclude.hlsli"

float4 main(WAVE_PARTICLE input) : SV_TARGET
{
    // return interpolated color
    return float4(input.velocity, input.amplitude, 1);
}
