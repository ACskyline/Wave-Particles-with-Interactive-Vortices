#define NUM_CONTROL_POINTS_INPUT 4
#define NUM_CONTROL_POINTS_OUTPUT 4
#define PI 3.14159265359
#define HALF_PI 1.57079632679

float2 BLERP2(float2 v00, float2 v01, float2 v10, float2 v11, float2 uv)
{
    return lerp(lerp(v00, v01, float2(uv.y, uv.y)), lerp(v10, v11, float2(uv.y, uv.y)), float2(uv.x, uv.x));
}

float3 BLERP3(float3 v00, float3 v01, float3 v10, float3 v11, float2 uv)
{
    return lerp(lerp(v00, v01, float3(uv.y, uv.y, uv.y)), lerp(v10, v11, float3(uv.y, uv.y, uv.y)), float3(uv.x, uv.x, uv.x));
}

/////////////// UNIFORM ///////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//

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
    float waveParticleScale;
    uint edgeTessFactor;
    uint insideTessFactor;
    uint textureWidth;
    uint textureHeight;
    uint blurRadius;
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
/////////////// UNIFORM ///////////////

///////////////// VS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct VS_INPUT
{
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct WAVE_PARTICLE
{
    float4 pos : SV_POSITION;
    float2 direction : DIRECTION;
    float height : HEIGHT;
    float speed : SPEED;
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
};
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// DS /////////////////


///////////////// PS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
SamplerState s0 : register(s0);
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// PS /////////////////