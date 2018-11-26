#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"

class Frame
{
public:
	struct FrameUniform
	{
		uint32_t time;
	};

	Frame();
	~Frame();

	void AddCamera(Camera* pCamera);
	void AddMesh(Mesh* pMesh);
	void AddTexture(Texture* pTexture);
	void AddRenderTexture(RenderTexture* pRenderTexture);

	void SetUniformTime(uint32_t time);
	uint32_t GetUniformTime();

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
