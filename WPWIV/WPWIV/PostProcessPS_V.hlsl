#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    //float4 sum = t1.Sample(s0, input.texCoord);
    float4 sum = float4(0, 0, 0, 0);
    
//    for (int i = -int(blurRadius); i <= int(blurRadius); i++)
//    {
////        float offset = float(i) / float(textureWidth);
////        float modifiedI = float(i) / float(blurRadius) * HALF_PI;
////        float weight = sin(modifiedI + HALF_PI);
        
////        float4 data = t1.Sample(s0, input.texCoord + float2(0, offset));
////        if(data.x > 0)
////        {
////#ifdef USE_RADIUS
////        if (abs(offset) < abs(data.y) * radiusScale)
////#endif
////            sum.rgb += weight * data.rgb;
////        }
        
//        //float offset = float(i) / float(textureWidth);
//        //float4 data = t1.Sample(s0, input.texCoord + float2(0, offset));
//        ////offset = abs(offset);
//        //if (data.x > 0 && data.y > abs(offset))//
//        //{
//        //    float x = -offset;
//        //    float r = data.y;
//        //    sum.rg += data.xy * 0.5 * (cos(PI * x / r) + 1);
//        //    sum.b += data.z * 0.25 * (cos(PI * x / r) + 1) * (cos(PI * x / r) + 1);
//        //    sum.a += data.w * -0.5 * sin(PI * x / r) * (cos(PI * x / r) + 1);
//        //}

//    }

//    return sum;

    float4 f = t1.Sample(s0, input.texCoord);
    float4 deviation = float4(f.y, 0, f.x, 1);
    for (int i = 1; i <= blurRadius; i++)
    {
        float offset = i / float(textureWidth);
        float4 fB = t1.Sample(s0, input.texCoord + float2(0, offset)); //f1, f4, f5
        float4 fT = t1.Sample(s0, input.texCoord + float2(0, -offset)); //f1, f4, f5
        float3 filter = GetFilter(i / float(blurRadius));
        deviation.x += (fB.y + fT.y) * filter.x * filter.x;
        deviation.y += (fB.z - fT.y) * 2 * filter.x * filter.y;
        deviation.z += (fB.x + fT.x) * filter.x;
    }

    return deviation;

}
