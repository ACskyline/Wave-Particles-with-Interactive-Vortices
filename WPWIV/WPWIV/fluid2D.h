#pragma once

#include "GlobalInclude.h"

//enum texOP {
//	OPadvection = 0,
//	OPbuoyancy = 1,
//	OPcomputedivergence = 2,
//	OPjacobi = 3,
//	OPsplat = 4,
//	OPsubtractgradient = 5
//};

struct FluidUniform
{
	const float AmbientTemperature = 0.0f;
	const float ImpulseDensity = 1.0f;
	const uint32_t NumJacobiIterations = 40;
	const float TimeStep = 0.00825f;
	const float SmokeBuoyancy = 1.0f;
	const float SmokeWeight = 0.05f;
	const float GradientScale = 1.125f / CellSize;
	const float TemperatureDissipation = 0.99f;
	const float VelocityDissipation = 0.99f;
	const float DensityDissipation = 0.9999f;
	XMFLOAT2 velocitystate = {0,1};
	XMFLOAT2 temperaturestate = { 0,1 };
	XMFLOAT2 pressurestate = { 0,1 };
	XMFLOAT2 densitystate = { 0,1 };
	XMFLOAT2 size = { 1000,1000 };
	const float cellsize = 1.125;
	uint32_t Curadvection = 0;//0 vel , 1 temp, 2 density
	uint32_t Curimpulse = 0;//0 temp ,1 density
};



class Fluid {
public:

	Fluid();
	~Fluid();

	bool initFluid(ID3D12Device* device);

	bool CreateFluidUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();
	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();
	void textureOPhandle();
	int gettextureOP();
	void resettextureOP();
	void swapvelstate();
	void swaptempstate();
	void swapprestate();
	void swapdenstate();
	int getvelstate();
	int gettempstate();
	int getprestate();
	int getdenstate();

	void setadvectionvel();
	void setadvectiontemp();
	void setadvectiondens();

	void setimpulsetemp();
	void setimpulsedens();
	void setimpulsevel();
protected:

	FluidUniform uniform;
	ID3D12Resource* gpuUniformBuffer;
	void* cpuUniformBufferAddress;
};