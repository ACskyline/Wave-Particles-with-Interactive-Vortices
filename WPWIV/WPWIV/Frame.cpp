#include "Frame.h"

Frame::Frame()
{
}

Frame::~Frame()
{
	ReleaseBuffer();
}

void Frame::AddCamera(Camera* pCamera)
{
	pCameraVec.push_back(pCamera);
}

void Frame::AddMesh(Mesh* pMesh)
{
	pMeshVec.push_back(pMesh);
}

void Frame::AddTexture(Texture* pTexture)
{
	pTextureVec.push_back(pTexture);
}

void Frame::AddRenderTexture(RenderTexture* pRenderTexture)
{
	pRenderTextureVec.push_back(pRenderTexture);
}

void Frame::SetUniformWaveParticleScale(float _waveParticleScale)
{
	uniform.waveParticleScale = _waveParticleScale;
}

void Frame::SetUniformEdgeTessFactor(uint32_t _tessellationFactor)
{
	uniform.edgeTessFactor = _tessellationFactor;
}

void Frame::SetUniformInsideTessFactor(uint32_t _tessellationFactor)
{
	uniform.insideTessFactor = _tessellationFactor;
}

void Frame::SetUniformTexutureWidthHeight(uint32_t texWidth, uint32_t texHeight)
{
	uniform.textureWidth = texWidth;
	uniform.textureHeight = texHeight;
}

void Frame::SetUniformBlurRadius(uint32_t blurR)
{
	uniform.blurRadius = blurR;
}

float Frame::GetUniformWaveParticleScale()
{
	return uniform.waveParticleScale;
}

uint32_t Frame::GetUniformEdgeTessFactor()
{
	return uniform.edgeTessFactor;
}

uint32_t Frame::GetUniformInsideTessFactor()
{
	return uniform.insideTessFactor;
}

uint32_t Frame::GetUniformBlurRadius()
{
	return uniform.blurRadius;
}

bool Frame::CreateUniformBuffer(ID3D12Device* device)
{

	HRESULT hr;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(FrameUniform)), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
		nullptr, // we do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&gpuUniformBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	gpuUniformBuffer->SetName(L"frame uniform buffer");

	CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

	// map the resource heap to get a gpu virtual address to the beginning of the heap
	hr = gpuUniformBuffer->Map(0, &readRange, &cpuUniformBufferAddress);

	return true;
}

void Frame::UpdateUniformBuffer()
{
	memcpy(cpuUniformBufferAddress, &uniform, sizeof(uniform));
}

void Frame::ReleaseBuffer()
{
	SAFE_RELEASE(gpuUniformBuffer);
}

D3D12_GPU_VIRTUAL_ADDRESS Frame::GetUniformBufferGpuAddress()
{
	return gpuUniformBuffer->GetGPUVirtualAddress();
}

bool Frame::InitFrame(ID3D12Device* device)
{
	if (!CreateUniformBuffer(device))
		return false;
	UpdateUniformBuffer();
	return true;
}

vector<Texture*>& Frame::GetTextureVec()
{
	return pTextureVec;
}

vector<RenderTexture*>& Frame::GetRenderTextureVec()
{
	return pRenderTextureVec;
}

vector<Camera*>& Frame::GetCameraVec()
{
	return pCameraVec;
}

vector<Mesh*>& Frame::GetMeshVec()
{
	return pMeshVec;
}