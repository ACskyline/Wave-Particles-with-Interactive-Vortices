#pragma once

#include "GlobalInclude.h"


struct FluidUniform
{
	const float AmbientTemperature = 0.0f;
	const float ImpulseTemperature = 10.0f;
	const float ImpulseDensity = 1.0f;
	const int NumJacobiIterations = 40;
	const float TimeStep = 0.125f;
	const float SmokeBuoyancy = 1.0f;
	const float SmokeWeight = 0.05f;
	const float GradientScale = 1.125f / CellSize;
	const float TemperatureDissipation = 0.99f;
	const float VelocityDissipation = 0.99f;
	const float DensityDissipation = 0.9999f;
	const XMFLOAT2 ImpulsePosition = { GridWidth / 2, -(int)SplatRadius / 2 };
	uint32_t increament = 0;
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
	void increamenthandle();
	int getincreament();
	void resetincreament();
protected:

	FluidUniform uniform;
	ID3D12Resource* gpuUniformBuffer;
	void* cpuUniformBufferAddress;
};