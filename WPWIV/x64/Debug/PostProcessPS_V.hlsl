#include "GlobalInclude.hlsli"

Texture2D horizontalFilter1 : register(t0);
Texture2D horizontalFilter2 : register(t1);
SamplerState wrapSampler : register(s0);

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
    output.col1 = float4(0, 0, 0, 0);
    output.col2 = float4(0, 0, 0, 0);

    float3 f123 = horizontalFilter1.Sample(wrapSampler, input.texCoord).xyz;
    float4 f45v = horizontalFilter2.Sample(wrapSampler, input.texCoord);
    float4 deviation = float4(f45v.x, 0, f123.x, 1); // initialize deviation at this pixel
    float4 gradient = float4(f123.y, 0, 0, 1); // initialize gradient at this pixel
    float2 gradCorr = float2(f123.z, f45v.y); // initialize gradient correction

    if(mode==0||mode==8||mode==9||mode==10||mode==11)
    {
        for (int i = 1; i <= blurRadius; i++)
        {
            float offset = i / float(textureHeight);

            float4 f123B = horizontalFilter1.Sample(wrapSampler, input.texCoord + float2(0, offset));
            float4 f123T = horizontalFilter1.Sample(wrapSampler, input.texCoord + float2(0, -offset));

            float4 f45vB = horizontalFilter2.Sample(wrapSampler, input.texCoord + float2(0, offset));
            float4 f45vT = horizontalFilter2.Sample(wrapSampler, input.texCoord + float2(0, -offset));

            float3 f = GetFilter(i / float(blurRadius));

            deviation.x += (f45vB.x + f45vT.x) * f.x * f.x; // deviation X
            deviation.y += (f45vB.y - f45vT.y) * 2 * f.x * f.y; // deviation Y
            deviation.z += (f123B.x + f123T.x) * f.x; // deviation Z
            gradient.x += (f123B.y + f123T.y) * f.x; // gradient X
            gradient.y += (f123B.x - f123T.x) * f.y; // gradient Y
            gradCorr.x += (f123B.z + f123T.z) * f.x * f.x; // gradient X horizontal deviation
            gradCorr.y += (f45vB.y + f45vT.y) * f.z; // gradient Y horizontal deviation

        }
        gradCorr *= PI / blurRadius;
        gradient.xy *= (PI / blurRadius) / (1 + gradCorr);

    }

    deviation.x *= dxScale;
    deviation.y *= dzScale;
    output.col1 = float4(-deviation.x, deviation.z, -deviation.y, deviation.w);
    output.col2 = gradient;

    return output;
}
