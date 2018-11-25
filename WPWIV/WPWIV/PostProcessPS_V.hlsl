#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    //float4 sum = t1.Sample(s0, input.texCoord);
    float4 sum = float4(0, 0, 0, 1.0);
    
    for (int i = -int(blurRadius); i <= int(blurRadius); i++)
    {
        float offset = float(i) / float(textureWidth);
        float modifiedI = float(i) / float(blurRadius) * HALF_PI;
        float weight = sin(modifiedI + HALF_PI);

        sum.rgb += weight * t1.Sample(s0, input.texCoord + float2(0, offset)).rgb;
    }

    return sum;
}