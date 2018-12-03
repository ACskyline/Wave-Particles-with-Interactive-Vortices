#pragma once

#include "Frame.h"
#include "Shader.h"

class Scene
{
public:
	struct SceneUniform
	{
		float heightScale;
		float waveParticleSpeedScale;
		float flowSpeed;
		float dxScale;
		float dzScale;
		float timeScale;
		float timeStepFluid;//
		float jacobiObstacleScale;//1
		float fluidCellSize;//1.25
		float jacobiInvBeta;//0.243
		float fluidDissipation;//1
		float gradientScale;//0.07 * 1.125
		float splatDirU;//
		float splatDirV;//
		float splatScale;//0.015
		uint32_t edgeTessFactor;
		uint32_t insideTessFactor;
		uint32_t textureWidth;
		uint32_t textureHeight;
		uint32_t textureWidthFluid;
		uint32_t textureHeightFluid;
		uint32_t blurRadius;
		uint32_t mode; 
		//0 - default, 
		//1 - flow map, 
		//2 - flow map driven texture, 
		//3 - wave particle, 
		//4 - horizontal blur, 
		//5 - vertical blur, 
		//6 - horizontal and vertical blur
		//7 - normal
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
	void SetUniformWaveParticleSpeedScale(float _waveParticleSpeedScale);
	void SetUniformFlowSpeed(float _flowSpeed);
	void SetUniformDxScale(float _dxScale);
	void SetUniformDzScale(float _dzScale);
	void SetUniformTimeScale(float _timeScale);
	void SetUniformTimeStepFluid(float _timeStepFluid);
	void SetUniformJacobiObstacleScale(float _jacobiObstacleScale);
	void SetUniformFluidCellSize(float _fluidCellSize);
	void SetUniformJacobiInvBeta(float _jacobiInvBeta);
	void SetUniformFluidDissipation(float _fluidDissipation);
	void SetUniformGradientScale(float _gradientScale);
	void SetUniformSplatDirU(float _splatDirU);
	void SetUniformSplatDirV(float _splatDirV);
	void SetUniformSplatScale(float _splatScale);
	void SetUniformEdgeTessFactor(uint32_t _tessellationFactor);
	void SetUniformInsideTessFactor(uint32_t _tessellationFactor);
	void SetUniformTextureWidthHeight(uint32_t texWidth, uint32_t texHeight);
	void SetUniformTextureWidthHeightFluid(uint32_t texWidth, uint32_t texHeight);
	void SetUniformBlurRadius(uint32_t _blurRadius);
	void SetUniformMode(uint32_t _mode);
	float GetUniformHeightScale();
	float GetUniformWaveParticleSpeedScale();
	float GetUniformFlowSpeed();
	float GetUniformDxScale();
	float GetUniformDzScale();
	float GetUniformTimeScale();
	float GetUniformTimeStepFluid();
	float GetUniformJacobiObstacleScale();
	float GetUniformFluidCellSize();
	float GetUniformJacobiInvBeta();
	float GetUniformFluidDissipation();
	float GetUniformGradientScale();
	float GetUniformSplatDirU();
	float GetUniformSplatDirV();
	float GetUniformSplatScale();
	uint32_t GetUniformEdgeTessFactor();
	uint32_t GetUniformInsideTessFactor();
	uint32_t GetUniformBlurRadius();
	uint32_t GetUniformMode();

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
