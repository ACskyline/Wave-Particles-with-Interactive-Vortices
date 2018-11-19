#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

class Scene
{
public:
	struct SceneUniform
	{
		float waveParticleScale;
	};

	Scene();
	~Scene();

	void SetWaveParticleScale(float _waveParticleScale);
	float GetWaveParticleScale();

	bool InitScene(ID3D12Device* device);
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();

	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();

	Camera* pCamera;
	vector<Mesh*> pMeshVec;
	vector<Texture*> pTextureVec;
	vector<Shader*> pShaderVec;

	SceneUniform uniform;
	ID3D12Resource* gpuUniformBuffer;
	void* cpuUniformBufferAddress;
};

