#include "GlobalInclude.hlsli"

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
    output.col1 = float4(0, 0, 0, 0);
    output.col2 = float4(0, 0, 0, 0);

    float3 f123 = t1.Sample(s0, input.texCoord).xyz;
    float4 f45v = t2.Sample(s0, input.texCoord);
    float4 deviation = float4(f45v.x, 0, f123.x, 1); // initialize deviation at this pixel
    float4 gradient = float4(f123.y, 0, 0, 1); // initialize gradient at this pixel
    float2 gradCorr = float2(f123.z, f45v.y); // initialize gradient correction

    if(mode==0||mode==5||mode==6||mode==7)
    {
        for (int i = 1; i <= blurRadius; i++)
        {
            float offset = i / float(textureHeight);

            float4 f123B = t1.Sample(s0, input.texCoord + float2(0, offset));
            float4 f123T = t1.Sample(s0, input.texCoord + float2(0, -offset));

            float4 f45vB = t2.Sample(s0, input.texCoord + float2(0, offset));
            float4 f45vT = t2.Sample(s0, input.texCoord + float2(0, -offset));

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
