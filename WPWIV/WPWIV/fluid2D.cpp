#include"fluid2D.h"

Fluid::Fluid()
{

}

Fluid::~Fluid()
{
	ReleaseBuffer();
}

bool Fluid::CreateFluidUniformBuffer(ID3D12Device* device)
{
	HRESULT hr;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(FluidUniform)), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
		nullptr, // we do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&gpuUniformBuffer));
	if (FAILED(hr))
	{
		return false;
	}

	gpuUniformBuffer->SetName(L"fluid uniform buffer");

	CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

	// map the resource heap to get a gpu virtual address to the beginning of the heap
	hr = gpuUniformBuffer->Map(0, &readRange, &cpuUniformBufferAddress);

	return true;
}

void Fluid::UpdateUniformBuffer()
{
	memcpy(cpuUniformBufferAddress, &uniform, sizeof(uniform));
}

void Fluid::textureOPhandle()
{
	uniform.textureOP++;
}

int Fluid::gettextureOP()
{
	return uniform.textureOP;
}

void Fluid::swapvelstate()
{
	int tmp = uniform.velocitystate.x;
	uniform.velocitystate.x = uniform.velocitystate.y;
	uniform.velocitystate.y = tmp;
}

void Fluid::swapprestate()
{
	int tmp = uniform.pressurestate.x;
	uniform.pressurestate.x = uniform.pressurestate.y;
	uniform.pressurestate.y = tmp;
}

void Fluid::swaptempstate()
{
	int tmp = uniform.temperaturestate.x;
	uniform.temperaturestate.x = uniform.temperaturestate.y;
	uniform.temperaturestate.y = tmp;
}

void Fluid::swapdenstate()
{
	int tmp = uniform.densitystate.x;
	uniform.densitystate.x = uniform.densitystate.y;
	uniform.densitystate.y = tmp;
}

int Fluid::getvelstate()
{
	return uniform.velocitystate.x;
}

int Fluid::gettempstate()
{
	return uniform.temperaturestate.x;
}

int Fluid::getprestate()
{
	return uniform.pressurestate.x;
}

int Fluid::getdenstate()
{
	return uniform.densitystate.x;
}

void Fluid::setadvectionvel()
{
	uniform.Curadvection = 0;
}


void Fluid::setadvectiontemp()
{
	uniform.Curadvection = 1;
}

void Fluid::setadvectiondens()
{
	uniform.Curadvection = 2;
}

void Fluid::setimpulsetemp()
{
	uniform.Curimpulse = 0;
}

void Fluid::setimpulsedens()
{
	uniform.Curimpulse = 1;
}

void Fluid::resettextureOP()
{
	uniform.textureOP = 0;
}

void Fluid::ReleaseBuffer()
{
	SAFE_RELEASE(gpuUniformBuffer);
}

D3D12_GPU_VIRTUAL_ADDRESS Fluid::GetUniformBufferGpuAddress()
{
	return gpuUniformBuffer->GetGPUVirtualAddress();
}

bool Fluid::initFluid(ID3D12Device* device)
{

	if (!CreateFluidUniformBuffer(device))
		return false;
	UpdateUniformBuffer();
	return true;
}