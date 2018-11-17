struct VS_INPUT
{
    float4 pos : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 wvpMat;
};

cbuffer CameraUniform : register(b1)
{
    float4x4 viewProj;
    float4x4 viewProjInv;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(mul(viewProj, wvpMat), input.pos); //mul(input.pos, mul(wvpMat, viewProj)); //mul(input.pos, wvpMat);
    output.texCoord = input.texCoord;
    return output;
}