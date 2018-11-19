#include "Scene.h"

Scene::Scene() :
	uniform({ 0.5, 32 })
{
}

Scene::~Scene()
{
	ReleaseBuffer();
}

void Scene::SetWaveParticleScale(float _waveParticleScale)
{
	uniform.waveParticleScale = _waveParticleScale;
}

void Scene::SetTessellationFactor(uint32_t _tessellationFactor)
{
	uniform.tessellationFactor = _tessellationFactor;
}

float Scene::GetWaveParticleScale()
{
	return uniform.waveParticleScale;
}

uint32_t Scene::GetTessellationFactor()
{
	return uniform.tessellationFactor;
}

bool Scene::CreateUniformBuffer(ID3D12Device* device)
{

	HRESULT hr;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneUniform)), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
		nullptr, // we do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&gpuUniformBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	gpuUniformBuffer->SetName(L"scene uniform buffer");

	CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

	// map the resource heap to get a gpu virtual address to the beginning of the heap
	hr = gpuUniformBuffer->Map(0, &readRange, &cpuUniformBufferAddress);

	return true;
}

void Scene::UpdateUniformBuffer()
{
	memcpy(cpuUniformBufferAddress, &uniform, sizeof(uniform));
}

void Scene::ReleaseBuffer()
{
	SAFE_RELEASE(gpuUniformBuffer);
}

D3D12_GPU_VIRTUAL_ADDRESS Scene::GetUniformBufferGpuAddress()
{
	return gpuUniformBuffer->GetGPUVirtualAddress();
}

bool Scene::InitScene(ID3D12Device* device)
{
	if (!CreateUniformBuffer(device))
		return false;
	UpdateUniformBuffer();
	return true;
}