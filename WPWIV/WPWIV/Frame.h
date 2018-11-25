#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"

class Frame
{
public:
	struct FrameUniform
	{
		float waveParticleScale;
		uint32_t edgeTessFactor;
		uint32_t insideTessFactor;
		uint32_t textureWidth;
		uint32_t textureHeight;
		uint32_t blurRadius;
	};

	Frame();
	~Frame();

	void AddCamera(Camera* pCamera);
	void AddMesh(Mesh* pMesh);
	void AddTexture(Texture* pTexture);
	void AddRenderTexture(RenderTexture* pRenderTexture);

	void SetUniformWaveParticleScale(float _waveParticleScale);
	void SetUniformEdgeTessFactor(uint32_t _tessellationFactor);
	void SetUniformInsideTessFactor(uint32_t _tessellationFactor);
	void SetUniformTexutureWidthHeight(uint32_t texWidth, uint32_t texHeight);
	void SetUniformBlurRadius(uint32_t blurR);
	float GetUniformWaveParticleScale();
	uint32_t GetUniformEdgeTessFactor();
	uint32_t GetUniformInsideTessFactor();
	uint32_t GetUniformBlurRadius();

	vector<Texture*>& GetTextureVec();
	vector<RenderTexture*>& GetRenderTextureVec();
	vector<Camera*>& GetCameraVec();
	vector<Mesh*>& GetMeshVec();

	bool InitFrame(ID3D12Device* device);
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();

	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();

private:
	vector<Camera*> pCameraVec;
	vector<Mesh*> pMeshVec;
	vector<Texture*> pTextureVec;
	vector<RenderTexture*> pRenderTextureVec;

	FrameUniform uniform;
	ID3D12Resource* gpuUniformBuffer;
	void* cpuUniformBufferAddress;
};
