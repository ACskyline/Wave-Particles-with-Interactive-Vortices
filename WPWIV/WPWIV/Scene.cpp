#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	ReleaseBuffer();
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

void Scene::SetUniformFoamScale(float _foamScale)
{
	uniform.foamScale = _foamScale;
}

void Scene::SetUniformTimeStepFluid(float _timeStepFluid)
{
	uniform.timeStepFluid = _timeStepFluid;
}

void Scene::SetUniformFluidCellSize(float _fluidCellSize)
{
	uniform.fluidCellSize = _fluidCellSize;
}

void Scene::SetUniformFluidDissipation(float _fluidDissipation)
{
	uniform.fluidDissipation = _fluidDissipation;
}

void Scene::SetUniformVorticityScale(float _vorticityScale)
{
	uniform.vorticityScale = _vorticityScale;
}

void Scene::SetUniformSplatDirU(float _splatDirU)
{
	uniform.splatDirU = _splatDirU;
}

void Scene::SetUniformSplatDirV(float _splatDirV)
{
	uniform.splatDirV = _splatDirV;
}

void Scene::SetUniformSplatScale(float _splatScale)
{
	uniform.splatScale = _splatScale;
}

void Scene::SetUniformSplatDensityU(float _splatU)
{
	uniform.splatDensityU = _splatU;
}

void Scene::SetUniformSplatDensityV(float _splatV)
{
	uniform.splatDensityV = _splatV;
}

void Scene::SetUniformSplatDensityRadius(float _splatR)
{
	uniform.splatDensityRadius = _splatR;
}

void Scene::SetUniformSplatDensityScale(float _splatScale)
{
	uniform.splatDensityScale = _splatScale;
}

void Scene::SetUniformBrushScale(float _brushScale)
{
	uniform.brushScale = _brushScale;
}

void Scene::SetUniformBrushStrength(float _brushStrength)
{
	uniform.brushStrength = _brushStrength;
}

void Scene::SetUniformBrushOffsetU(float _brushOffsetU)
{
	uniform.brushOffsetU = _brushOffsetU;
}

void Scene::SetUniformBrushOffsetV(float _brushOffsetV)
{
	uniform.brushOffsetV = _brushOffsetV;
}

void Scene::SetUniformObstacleScale(float _obstacleScale)
{
	uniform.obstacleScale = _obstacleScale;
}

void Scene::SetUniformObstacleThresholdFluid(float _obstacleThresholdFluid)
{
	uniform.obstacleThresholdFluid = _obstacleThresholdFluid;
}

void Scene::SetUniformObstacleThresholdWave(float _obstacleThresholdWave)
{
	uniform.obstacleThresholdWave = _obstacleThresholdWave;
}

void Scene::SetUniformEdgeTessFactor(uint32_t _tessellationFactor)
{
	uniform.edgeTessFactor = _tessellationFactor;
}

void Scene::SetUniformInsideTessFactor(uint32_t _tessellationFactor)
{
	uniform.insideTessFactor = _tessellationFactor;
}

void Scene::SetUniformTextureWidthHeight(uint32_t texWidth, uint32_t texHeight)
{
	uniform.textureWidth = texWidth;
	uniform.textureHeight = texHeight;
}

void Scene::SetUniformTextureWidthHeightFluid(uint32_t texWidth, uint32_t texHeight)
{
	uniform.textureWidthFluid = texWidth;
	uniform.textureHeightFluid = texHeight;
}

void Scene::SetUniformTextureWidthFluid(uint32_t texWidth)
{
	uniform.textureWidthFluid = texWidth;
}

void Scene::SetUniformTextureHeightFluid(uint32_t texHeight)
{
	uniform.textureHeightFluid = texHeight;
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

float Scene::GetUniformFoamScale()
{
	return uniform.foamScale;
}

float Scene::GetUniformTimeStepFluid()
{
	return uniform.timeStepFluid;
}

float Scene::GetUniformFluidCellSize()
{
	return uniform.fluidCellSize;
}

float Scene::GetUniformFluidDissipation()
{
	return uniform.fluidDissipation;
}

float Scene::GetUniformVorticityScale()
{
	return uniform.vorticityScale;
}

float Scene::GetUniformSplatDirU()
{
	return uniform.splatDirU;
}

float Scene::GetUniformSplatDirV()
{
	return uniform.splatDirV;
}

float Scene::GetUniformSplatScale()
{
	return uniform.splatScale;
}

float Scene::GetUniformSplatDensityU()
{
	return uniform.splatDensityU;
}

float Scene::GetUniformSplatDensityV()
{
	return uniform.splatDensityV;
}

float Scene::GetUniformSplatDensityRadius()
{
	return uniform.splatDensityRadius;
}

float Scene::GetUniformSplatDensityScale()
{
	return uniform.splatDensityScale;
}

float Scene::GetUniformBrushScale()
{
	return uniform.brushScale;
}

float Scene::GetUniformBrushStrength()
{
	return uniform.brushStrength;
}

float Scene::GetUniformBrushOffsetU()
{
	return uniform.brushOffsetU;
}

float Scene::GetUniformBrushOffsetV()
{
	return uniform.brushOffsetV;
}

float Scene::GetUniformObstacleScale()
{
	return uniform.obstacleScale;
}

float Scene::GetUniformObstacleThresholdFluid()
{
	return uniform.obstacleThresholdFluid;
}

float Scene::GetUniformObstacleThresholdWave()
{
	return uniform.obstacleThresholdWave;
}

uint32_t Scene::GetUniformTextureWidthFluid()
{
	return uniform.textureWidthFluid;
}

uint32_t Scene::GetUniformTextureHeightFluid()
{
	return uniform.textureHeightFluid;
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



float Scene::GetUniformLightHight()
{
	return uniform.lighthight;
}

float Scene::GetUniformExtinctcoeff()
{
	return uniform.extinctcoeff;
}

float Scene::GetShiness()
{
	return uniform.shiness;
}

float Scene::GetfScale()
{
	return uniform.fscale;
}

float Scene::GetFpow()
{
	return uniform.fpow;
}

float Scene::GetBias()
{
	return uniform.fbias;
}

float Scene::GetFoamScale()
{
	return uniform.foamScale;
}

void Scene::SetUniformLightHight(float v)
{
	uniform.lighthight = v;
}

void Scene::SetUniformExtinctcoeff(float v)
{
	uniform.extinctcoeff = v;
}

void Scene::SetShiness(float v)
{
	uniform.shiness = v;
}

void Scene::SetFScale(float v)
{
	uniform.fscale = v;
}

void Scene::SetFPow(float v)
{
	uniform.fpow = v;
}

void Scene::SetBias(float v)
{
	uniform.fbias = v;
}

void Scene::SetFoamScale(float v)
{
	uniform.FoamScale = v;
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
			printf("LoadTexture %d: %ls failed\n", i, pTextureVec[i]->GetName().c_str());
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

void Scene::Release()
{
	for (auto frame : pFrameVec)
	{
		frame->ReleaseBuffer();
	}

	for (auto camera : pCameraVec)
	{
		camera->ReleaseBuffer();
	}

	for (auto mesh : pMeshVec)
	{
		mesh->ReleaseBuffers();
	}

	for (auto texture : pTextureVec)
	{
		texture->ReleaseBuffer();
		texture->ReleaseBufferCPU();
	}

	for (auto shader : pShaderVec)
	{
		shader->ReleaseBuffer();
	}

	for (auto renderTexture : pRenderTextureVec)
	{
		renderTexture->ReleaseBuffers();
	}

	ReleaseBuffer();
}

D3D12_GPU_VIRTUAL_ADDRESS Scene::GetUniformBufferGpuAddress()
{
	return gpuUniformBuffer->GetGPUVirtualAddress();
}
