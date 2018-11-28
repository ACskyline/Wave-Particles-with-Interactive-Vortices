#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 f = t1.Sample(s0, input.texCoord); //horizontal 1, vertical, horizontal 2
    float4 deviation = float4(f.x, f.y, 0, 1);
    if(mode==0||mode==5||mode==6||mode==7)
    {
        for (int i = 1; i <= blurRadius; i++)
        {
            float offset = i / float(textureHeight);
            float4 fB = t1.Sample(s0, input.texCoord + float2(0, offset)); //horizontal 1, vertical, horizontal 2
            float4 fT = t1.Sample(s0, input.texCoord + float2(0, -offset)); //horizontal 1, vertical, horizontal 2
            float3 filter = GetFilter(i / float(blurRadius));
            deviation.x += (fB.x + fT.x) * filter.x * filter.x; //horizontal 1
            deviation.y += (fB.y + fT.y) * filter.x; //vertical
            deviation.z += (fB.z - fT.z) * 2 * filter.x * filter.y; //horizontal 2
        }
    }
    return deviation;
}
