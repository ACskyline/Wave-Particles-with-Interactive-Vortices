#pragma once

#include "Frame.h"
#include "Shader.h"

class Scene
{
public:
	struct SceneUniform
	{
		float heightScale;
		float radiusScale;
		float speedScale;
		float dxScale;
		float dzScale;
		float timeScale;
		uint32_t edgeTessFactor;
		uint32_t insideTessFactor;
		uint32_t textureWidth;
		uint32_t textureHeight;
		uint32_t blurRadius;
	};

	Scene();
	~Scene();
	void AddFrame(Frame* pFrame);
	void AddCamera(Camera* pCamera);
	void AddMesh(Mesh* pMesh);
	void AddTexture(Texture* pTexture);
	void AddShader(Shader* pShader);
	void AddRenderTexture(RenderTexture* pRenderTexture);

	void SetUniformHeightScale(float _heightScale);
	void SetUniformRadiusScale(float _radiusScale);
	void SetUniformSpeedScale(float _speedScale);
	void SetUniformDxScale(float _dxScale);
	void SetUniformDzScale(float _dzScale);
	void SetUniformTimeScale(float _timeScale);
	void SetUniformEdgeTessFactor(uint32_t _tessellationFactor);
	void SetUniformInsideTessFactor(uint32_t _tessellationFactor);
	void SetUniformTexutureWidthHeight(uint32_t texWidth, uint32_t texHeight);
	void SetUniformBlurRadius(uint32_t blurR);
	float GetUniformHeightScale();
	float GetUniformRadiusScale();
	float GetUniformSpeedScale();
	float GetUniformDxScale();
	float GetUniformDzScale();
	float GetUniformTimeScale();
	uint32_t GetUniformEdgeTessFactor();
	uint32_t GetUniformInsideTessFactor();
	uint32_t GetUniformBlurRadius();

	bool LoadScene();
	bool InitScene(ID3D12Device* device);
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();

	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();

private:
	vector<Frame*> pFrameVec;
	vector<Camera*> pCameraVec;
	vector<Mesh*> pMeshVec;
	vector<Texture*> pTextureVec;
	vector<Shader*> pShaderVec;
	vector<RenderTexture*> pRenderTextureVec;

	SceneUniform uniform;
	ID3D12Resource* gpuUniformBuffer;
	void* cpuUniformBufferAddress;
};
