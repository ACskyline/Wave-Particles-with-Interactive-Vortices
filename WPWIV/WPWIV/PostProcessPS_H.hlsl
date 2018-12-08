#include "GlobalInclude.hlsli"

Texture2D waveParticle : register(t0);
SamplerState wrapSampler : register(s0);

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
    output.col1 = float4(0, 0, 0, 0);
    output.col2 = float4(0, 0, 0, 0);

    float3 velAmp = waveParticle.Sample(wrapSampler, input.texCoord).xyz;
    float4 f123 = float4(velAmp.z, 0, 0.5 * velAmp.z, 1);
    float4 f45v = float4(0, velAmp.z, sign(velAmp.z) * velAmp.xy);

    if(mode==0||mode==7||mode==9||mode==10||mode==11)
    {
        for (int i = 1; i <= blurRadius; i++)
        {
            float offset = i / float(textureWidth);
            float4 velAmpL = waveParticle.Sample(wrapSampler, input.texCoord + float2(offset, 0));
            float4 velAmpR = waveParticle.Sample(wrapSampler, input.texCoord + float2(-offset, 0));
            float ampSum = velAmpL.z + velAmpR.z;
            float ampDif = velAmpL.z - velAmpR.z;
            float3 f = GetFilter(i / float(blurRadius));
            f123.x += ampSum * f.x;
            f123.y += ampDif * f.y;
            f123.z += ampSum * f.z;
            f45v.x += ampDif * f.x * f.y * 2;
            f45v.y += ampSum * f.x * f.x;
            
            f45v.z += (sign(velAmpL.z) * velAmpL.x + sign(velAmpR.z) * velAmpR.x) * f.x;
            f45v.w += (sign(velAmpL.z) * velAmpL.y + sign(velAmpR.z) * velAmpR.y) * f.x;
        }
    }

    output.col1 = f123;
    output.col2 = f45v;
    return output;
}
