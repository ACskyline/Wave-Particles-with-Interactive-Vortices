///////////////// VS /////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct VS_INPUT
{
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VSQuadOut {
	float4 position : SV_Position;
	float2 uv: TexCoord;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float3 pos : MODELPOS;
    float2 texCoord : TEXCOORD;
};

cbuffer ObjectUniform : register(b0)
{
    float4x4 model;
    float4x4 modelInv;
};

cbuffer CameraUniform : register(b1)
{
    float4x4 viewProj;
    float4x4 viewProjInv;
}

cbuffer FrameUniform : register(b2)
{
    float waveParticleScale;
    uint tessellationFactor;
}

cbuffer FluidUniform : register(b3)
{
	float AmbientTemperature;
	  float ImpulseTemperature;
	  float ImpulseDensity;
	  int NumJacobiIterations;
	  float TimeStep;
	  float SmokeBuoyancy;
	  float SmokeWeight;
	  float GradientScale;
	  float TemperatureDissipation;
	  float VelocityDissipation;
	  float DensityDissipation;
	  float2 ImpulsePosition;
	  uint increament;
}
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
    float EdgeTessFactor[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 3
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
Texture2D t2 : register(t2);
SamplerState s0 : register(s0);
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
///////////////// PS /////////////////