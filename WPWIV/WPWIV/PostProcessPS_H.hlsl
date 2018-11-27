#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{    
    float4 data = t0.Sample(s0, input.texCoord);
    float4 f = float4(0, data.x, data.x, 1); //height, 0, height
    for (int i = 1; i <= blurRadius; i++)
    {
        float offset = i / float(textureWidth);
        float4 dataL = t0.Sample(s0, input.texCoord + float2(-offset, 0)); //height, radius, direction
        float4 dataR = t0.Sample(s0, input.texCoord + float2(offset, 0)); //height, radius, direction
        float heightSum = dataR.x + dataL.x;
        float heightDif = dataR.x - dataL.x;
        float3 filter = GetFilter(i / float(blurRadius));
        f.x += heightDif * filter.x * filter.y * 2;//horizontal 1
        f.y += heightSum * filter.x;//vertical
        f.z += heightSum * filter.x * filter.x;//horizontal 2
    }

    return f;
}
