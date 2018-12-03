#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::AddFrame(Frame* pFrame)
{
	pFrameVec.push_back(pFrame);
}

void Scene::AddCamera(Camera* pCamera)
{
	pCameraVec.push_back(pCamera);
}

void Scene::AddMesh(Mesh* pMesh)
{
	pMeshVec.push_back(pMesh);
}

void Scene::AddTexture(Texture* pTexture)
{
	pTextureVec.push_back(pTexture);
}

void Scene::AddShader(Shader* pShader)
{
	pShaderVec.push_back(pShader);
}

void Scene::AddRenderTexture(RenderTexture* pRenderTexture)
{
	pRenderTextureVec.push_back(pRenderTexture);
}

void Scene::SetUniformHeightScale(float _heightScale)
{
	uniform.heightScale = _heightScale;
}

void Scene::SetUniformWaveParticleSpeedScale(float _waveParticleSpeedScale)
{
	uniform.waveParticleSpeedScale = _waveParticleSpeedScale;
}

void Scene::SetUniformFlowSpeed(float _flowSpeed)
{
	uniform.flowSpeed = _flowSpeed;
}

void Scene::SetUniformDxScale(float _dxScale)
{
	uniform.dxScale = _dxScale;
}

void Scene::SetUniformDzScale(float _dzScale)
{
	uniform.dzScale = _dzScale;
}

void Scene::SetUniformTimeScale(float _timeScale)
{
	uniform.timeScale = _timeScale;
}

void Scene::SetUniformEdgeTessFactor(uint32_t _tessellationFactor)
{
	uniform.edgeTessFactor = _tessellationFactor;
}

void Scene::SetUniformInsideTessFactor(uint32_t _tessellationFactor)
{
	uniform.insideTessFactor = _tessellationFactor;
}

void Scene::SetUniformTexutureWidthHeight(uint32_t texWidth, uint32_t texHeight)
{
	uniform.textureWidth = texWidth;
	uniform.textureHeight = texHeight;
}

void Scene::SetUniformBlurRadius(uint32_t blurRadius)
{
	uniform.blurRadius = blurRadius;
}

void Scene::SetUniformMode(uint32_t mode)
{
	uniform.mode = mode;
}

float Scene::GetUniformHeightScale()
{
	return uniform.heightScale;
}

float Scene::GetUniformWaveParticleSpeedScale()
{
	return uniform.waveParticleSpeedScale;
}

float Scene::GetUniformFlowSpeed()
{
	return uniform.flowSpeed;
}

float Scene::GetUniformDxScale()
{
	return uniform.dxScale;
}

float Scene::GetUniformDzScale()
{
	return uniform.dzScale;
}

float Scene::GetUniformTimeScale()
{
	return uniform.timeScale;
}

uint32_t Scene::GetUniformEdgeTessFactor()
{
	return uniform.edgeTessFactor;
}

uint32_t Scene::GetUniformInsideTessFactor()
{
	return uniform.insideTessFactor;
}

uint32_t Scene::GetUniformBlurRadius()
{
	return uniform.blurRadius;
}

uint32_t Scene::GetUniformMode()
{
	return uniform.mode;
}

float Scene::GetUniformLighthight()
{
	return uniform.lighthight;
}

void Scene::SetUniformLighthight(float v)
{
	uniform.lighthight = v;
}

float Scene::Getextinctcoeff()
{
	return uniform.extinctcoeff;
}

void Scene::Setextinctcoeff(float v)
{
	uniform.extinctcoeff = v;
}

float Scene::Getshiness()
{
	return uniform.shiness;
}

void Scene::Setshiness(float v)
{
	uniform.shiness = v;
}

bool Scene::LoadScene()
{
	// CPU side, load data from disk
	int shaderCount = pShaderVec.size();
	for (int i = 0; i < shaderCount; i++)
	{
		if (!pShaderVec[i]->CreateShader())
		{
			printf("CreateShader %d failed\n", i);
			return false;
		}
	}

	int textureCount = pTextureVec.size();
	for (int i = 0; i < textureCount; i++)
	{
		if (!pTextureVec[i]->LoadTextureBuffer())
		{
			printf("LoadTexture failed\n");
			return false;
		}
	}

	return true;
}

bool Scene::InitScene(ID3D12Device* device)
{
	int frameCount = pFrameVec.size();
	for (int i = 0; i < frameCount; i++)
	{
		if (!pFrameVec[i]->InitFrame(device))
		{
			printf("InitFrame failed\n");
			return false;
		}
	}

	int cameraCount = pCameraVec.size();
	for (int i = 0; i < cameraCount; i++)
	{
		if (!pCameraVec[i]->InitCamera(device))
		{
			printf("InitCamera failed\n");
			return false;
		}
	}

	int meshCount = pMeshVec.size();
	for (int i = 0; i < meshCount; i++)
	{
		if (!pMeshVec[i]->InitMesh(device))
		{
			printf("InitMesh failed\n");
			return false;
		}
	}

	int textureCount = pTextureVec.size();
	for (int i = 0; i < textureCount; i++)
	{
		if (!pTextureVec[i]->InitTexture(device))
		{
			printf("InitTexture failed\n");
			return false;
		}
	}

	int renderTextureCount = pRenderTextureVec.size();
	for (int i = 0; i < renderTextureCount; i++)
	{
		if (!pRenderTextureVec[i]->InitTexture(device))
		{
			printf("InitRenderTexture failed\n");
			return false;
		}
	}

	if (!CreateUniformBuffer(device))
		return false;
	UpdateUniformBuffer();

	return true;
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
