#include "GlobalInclude.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // return interpolated color
    //float4 sum = t0.Sample(s0, input.texCoord);
//    float4 sum = float4(0, 0, 0, 0);
    
//    for (int i = -int(blurRadius); i <= int(blurRadius); i++)
//    {
//        //float offset = float(i) / float(textureWidth);
//        //float modifiedI = float(i) / float(blurRadius) * HALF_PI;
//        //float weight = sin(modifiedI + HALF_PI);

//        ////sum.rgb += weight * t0.Sample(s0, input.texCoord + float2(offset, 0)).rgb;
        
////        float4 data = t0.Sample(s0, input.texCoord + float2(offset, 0));
////        if (data.x > 0)
////        {
////#ifdef USE_RADIUS
////            if (abs(offset) < abs(data.y) * radiusScale)
////#endif
////            sum.rgb += weight * data.rgb;
////        }

        //}
    //return sum;

    float4 sum = float4(0, 0, 0, 0);
    
    for (int i = -int(blurRadius); i <= int(blurRadius); i++)
    {
        float offset = float(i) / float(textureWidth);
        float4 data = t0.Sample(s0, input.texCoord + float2(offset, 0));
        //data.y = 0.5;
        data.z = 0.04 * dxScale; //        abs(data.z) 
        data.w = 0.04 * dzScale; //        abs(data.w) 
        if (data.x > 0 && data.y > abs(offset))//
        {
            float x = -offset; //from current point to wave particle
            float r = data.y;
            sum.rg += data.xy * 0.5 * (cos(PI * x / r) + 1);
            sum.b += data.z * -0.5 * sin(PI * x / r) * (cos(PI * x / r) + 1);
            sum.a += data.w * 0.25 * (cos(PI * x / r) + 1) * (cos(PI * x / r) + 1);
        }
        
    }

    return sum;
    
    //float4 data = t0.Sample(s0, input.texCoord);
    //float4 f = float4(data.x, 0, data.x, 1); //height, 0, height
    //for (int i = 1; i <= blurRadius; i++)
    //{
    //    float offset = i / float(textureWidth);
    //    float4 dataL = t0.Sample(s0, input.texCoord + float2(-offset, 0)); //height, radius, direction
    //    float4 dataR = t0.Sample(s0, input.texCoord + float2(offset, 0)); //height, radius, direction
    //    float heightSum = dataR.x + dataL.x;
    //    float heightDif = dataR.x - dataL.x;
    //    float3 filter = GetFilter(i / float(blurRadius));
    //    f.x += heightSum * filter.x;
    //    f.y += heightDif * filter.x * filter.y * 2;
    //    f.z += heightSum * filter.x * filter.x;
    //}

    //return f;
}
