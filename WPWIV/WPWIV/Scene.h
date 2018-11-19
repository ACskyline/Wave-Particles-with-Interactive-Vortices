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
		uint32_t tessellationFactor;
	};

	Scene();
	~Scene();

	void SetWaveParticleScale(float _waveParticleScale);
	void SetTessellationFactor(uint32_t _tessellationFactor);
	float GetWaveParticleScale();
	uint32_t GetTessellationFactor();

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

