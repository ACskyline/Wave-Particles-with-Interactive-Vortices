#include "GlobalInclude.hlsli"

float4 main(DS_OUTPUT input) : SV_TARGET
{
    if (mode == 0)//0 - default,
    {
        return t0.Sample(s1, input.texCoord);
    }
    else if (mode == 1)//1 - flow map, 
    {
        return t1.Sample(s1, input.texCoord);
    }
    else if (mode == 2)//2 - flow map driven texture, 
    {
        return Flow(input.texCoord, time * timeScale * flowSpeed, t1, s2, t0, s2);
    }
    else if (mode==3||mode==4||mode==5||mode==6)//3, 4, 5, 6 - blur
    {
        return t2.Sample(s1, input.texCoord);
    }
    else if (mode == 7)//7 - normal
    {
        //float3 normal = input.nor; //PER VERTEX NORMAL

        //PER PIXEL NORMAL
        //MODEL SPACE
        float3 normal = FlowHeightForNormal(input.texCoord, time * timeScale * flowSpeed, t1, s2, t2, s2);
        //WORLD SPACE, THIS IS IMPORTANT TO KEEP NORMAL CORRECT WITH LARGE SCALE WATER SURFACE
        //RIGHT MULTIPLY THE TRANSPOSE OF AN INVERSE MATRIX IS LEFT MULTIPLY THE INVERSE MATRIX
        //ADDITIONALY MATRIX PASSED FROM CPU TO GPU WILL TRANSPOSE AGAIN, SO IT CANCELED OUT
        normal = normalize(mul(float4(normal, 0), modelInv));
        return float4((normal + 1.0) * 0.5, 1.0);//[-1, 1] to [0, 1] for display purpose
    }

    return float4(0, 0, 0, 1);
}
