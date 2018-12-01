#define NUM_CONTROL_POINTS_INPUT 4
#define NUM_CONTROL_POINTS_OUTPUT 4
#define PI 3.14159265359
#define HALF_PI 1.57079632679

/////////////// UNIFORM ///////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//

//
//    |  wave_particle |   post_process  |    graphics   |
// t0 |      n/a       |   WaveParticle  |    Albedo     |
// t1 |      n/a       |   PostProcessH1 |    Flowmap    |
// t2 |      n/a       |   PostProcessH2 | PostProcessV1 |
// t3 |      n/a       |      n/a        | PostProcessV2 |
//

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);

//
//    |  wave_particle |   post_process  |    graphics   |
// s0 |border(not used)|      wrap       |     border    |
// s1 |      n/a       |      n/a        |     clamp     |
// s2 |      n/a       |      n/a        |     wrap      |
//

SamplerState s0 : register(s0);
SamplerState s1 : register(s1);
SamplerState s2 : register(s2);

cbuffer ObjectUniform : register(b0)
{
    float4x4 model;
    float4x4 modelInv;
};

cbuffer CameraUniform : register(b1)
{
    float4x4 viewProj;
    float4x4 viewProjInv;
};

cbuffer FrameUniform : register(b2)
{
    uint time;
};

cbuffer SceneUniform : register(b3)
{
    float heightScale;
    float waveParticleSpeedScale;
    float flowSpeed;
    float dxScale;
    float dzScale;
    float timeScale;
    uint edgeTessFactor;
    uint insideTessFactor;
    uint textureWidth;
    uint textureHeight;
    uint blurRadius;
    uint mode; 
    //0 - default, 
    //1 - flow map, 
    //2 - flow map driven texture, 
    //3 - wave particle, 
    //4 - horizontal blur, 
    //5 - vertical blur, 
    //6 - horizontal and vertical blur,
    //7 - normal
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
/////////////// UNIFORM ///////////////

///////////////// HELPER FUNCTION /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
float2 BLERP2(float2 v00, float2 v01, float2 v10, float2 v11, float2 uv)
{
    return lerp(lerp(v00, v01, float2(uv.y, uv.y)), lerp(v10, v11, float2(uv.y, uv.y)), float2(uv.x, uv.x));
}

float3 BLERP3(float3 v00, float3 v01, float3 v10, float3 v11, float2 uv)
{
    return lerp(lerp(v00, v01, float3(uv.y, uv.y, uv.y)), lerp(v10, v11, float3(uv.y, uv.y, uv.y)), float3(uv.x, uv.x, uv.x));
}

float3 GetFilter(float v)
{
    float s, c;
    sincos(PI * v, s, c);
    return float3(
        0.5f * (c + 1.0f), // 0.5 ( cos(v) + 1 )
        -0.5f * s, // -0.5 sin(v)
        -0.25f * (c * c - s * s + c) // cos(2v) + cos(v)
    );
}

float4 Flow(in float2 uv, in float time, in Texture2D flowT, in SamplerState flowS, in Texture2D flowedT, in SamplerState flowedS)
{
    float timeInt = float(time) / (1 * 2); //interval is always 1
    float2 fTime = frac(float2(timeInt, timeInt + .5));
    float4 flowMap = flowT.SampleLevel(flowS, uv, 0);
    float2 flowDir = (flowMap.xy - float2(0.5, 0.5)) * 2;
    float2 flowUV1 = uv - (flowDir / 2) + fTime.x * flowDir.xy;
    float2 flowUV2 = uv - (flowDir / 2) + fTime.y * flowDir.xy;
    float4 tx1 = flowedT.SampleLevel(flowedS, flowUV1, 0);
    float4 tx2 = flowedT.SampleLevel(flowedS, flowUV2, 0);
    return lerp(tx1, tx2, abs(2 * frac(timeInt) - 1));
}

float3 HeightMapToNormal(float2 uv, Texture2D heightMapT, SamplerState heightMapS)
{
    float2 dUP = uv + float2(1.0 / textureWidth, 0);
    float2 dUN = uv - float2(1.0 / textureWidth, 0);
    float2 dVP = uv + float2(0, 1.0 / textureHeight);
    float2 dVN = uv - float2(0, 1.0 / textureHeight);

    float4 deviationUP = heightMapT.SampleLevel(heightMapS, dUP, 0);
    float4 deviationUN = heightMapT.SampleLevel(heightMapS, dUN, 0);
    float4 deviationVP = heightMapT.SampleLevel(heightMapS, dVP, 0);
    float4 deviationVN = heightMapT.SampleLevel(heightMapS, dVN, 0);
    
    float3 ddV = deviationVP.xyz - deviationVN.xyz;
    ddV += float3(0, 0, 2.0 / textureHeight);
    float3 ddU = deviationUP.xyz - deviationUN.xyz;
    ddU += float3(2.0 / textureWidth, 0, 0);

    return cross(normalize(ddV), normalize(ddU));
}

float4 FlowHeightWithNormal(in float2 uv, in float time, in Texture2D flowT, in SamplerState flowS, in Texture2D flowedT, in SamplerState flowedS, out float3 normal)
{
    float timeInt = float(time) / (1 * 2); //interval is always 1
    float2 fTime = frac(float2(timeInt, timeInt + .5));
    float4 flowMap = flowT.SampleLevel(flowS, uv, 0);
    float2 flowDir = (flowMap.xy - float2(0.5, 0.5)) * 2;
    float2 flowUV1 = uv - (flowDir / 2) + fTime.x * flowDir.xy;
    float2 flowUV2 = uv - (flowDir / 2) + fTime.y * flowDir.xy;
    float4 tx1 = flowedT.SampleLevel(flowedS, flowUV1, 0);
    float3 nor1 = HeightMapToNormal(flowUV1, flowedT, flowedS);
    float4 tx2 = flowedT.SampleLevel(flowedS, flowUV2, 0);
    float3 nor2 = HeightMapToNormal(flowUV2, flowedT, flowedS);
    normal = (lerp(nor1, nor2, abs(2 * frac(timeInt) - 1)));
    return lerp(tx1, tx2, abs(2 * frac(timeInt) - 1));
}

float3 FlowHeightForNormal(float2 uv, float time, Texture2D flowT, SamplerState flowS, Texture2D flowedT, SamplerState flowedS)
{
    float timeInt = float(time) / (1 * 2); //interval is always 1
    float2 fTime = frac(float2(timeInt, timeInt + .5));
    float4 flowMap = flowT.SampleLevel(flowS, uv, 0);
    float2 flowDir = (flowMap.xy - float2(0.5, 0.5)) * 2;
    float2 flowUV1 = uv - (flowDir / 2) + fTime.x * flowDir.xy;
    float2 flowUV2 = uv - (flowDir / 2) + fTime.y * flowDir.xy;
    float3 nor1 = HeightMapToNormal(flowUV1, flowedT, flowedS);
    float3 nor2 = HeightMapToNormal(flowUV2, flowedT, flowedS);
    return normalize(lerp(nor1, nor2, abs(2 * frac(timeInt) - 1)));
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// HELPER FUNCTION /////////////////

///////////////// VS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct VS_INPUT
{
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD;
    float3 nor : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct WAVE_PARTICLE
{
    float4 pos : SV_POSITION;
    float2 velocity : VELOCITY;
    float amplitude : AMPLITUDE;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float3 pos : MODELPOS;
    float2 texCoord : TEXCOORD;
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// VS /////////////////


///////////////// HS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct HS_CONTROL_POINT_OUTPUT
{
    float3 pos : MODELPOS;
    float2 texCoord : TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// HS /////////////////


///////////////// DS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    //float3 nor : NORMAL; //PER VERTEX NORMAL
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// DS /////////////////


///////////////// PS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct PS_OUTPUT
{
    float4 col1 : SV_TARGET0;
    float4 col2 : SV_TARGET1;
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// PS /////////////////
