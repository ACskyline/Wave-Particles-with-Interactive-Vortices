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
		float foamScale;

		float timeStepFluid;
		float fluidCellSize;
		float fluidDissipation;
		float vorticityScale;
		float splatDirU;
		float splatDirV;
		float splatScale;
		float splatDensityU;
		float splatDensityV;
		float splatDensityRadius;
		float splatDensityScale;

		float brushScale;
		float brushStrength;
		float brushOffsetU;
		float brushOffsetV;

		float obstacleScale;
		float obstacleThresholdFluid;
		float obstacleThresholdWave;

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
		//2 - density,
		//3 - divergence,
		//4 - pressure,
		//5 - flow map driven texture, 
		//6 - wave particle, 
		//7 - horizontal blur, 
		//8 - vertical blur, 
		//9 - horizontal and vertical blur,
		//10 - normal

		float lighthight;
		float extinctcoeff;
		float shiness;
		float fscale;
		float fpow;
		float fbias;
		float FoamScale;
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
	void SetUniformFoamScale(float _foamScale);

	void SetUniformTimeStepFluid(float _timeStepFluid);
	void SetUniformFluidCellSize(float _fluidCellSize);
	void SetUniformFluidDissipation(float _fluidDissipation);
	void SetUniformVorticityScale(float _vorticityScale);
	void SetUniformSplatDirU(float _splatDirU);
	void SetUniformSplatDirV(float _splatDirV);
	void SetUniformSplatScale(float _splatScale);
	void SetUniformSplatDensityU(float _splatU);
	void SetUniformSplatDensityV(float _splatV);
	void SetUniformSplatDensityRadius(float _splatR);
	void SetUniformSplatDensityScale(float _splatScale);

	void SetUniformBrushScale(float _brushScale);
	void SetUniformBrushStrength(float _brushStrength);
	void SetUniformBrushOffsetU(float _brushOffsetU);
	void SetUniformBrushOffsetV(float _brushOffsetV);

	void SetUniformObstacleScale(float _obstacleScale);
	void SetUniformObstacleThresholdFluid(float _obstacleThresholdFluid);
	void SetUniformObstacleThresholdWave(float _obstacleThresholdWave);

	void SetUniformEdgeTessFactor(uint32_t _tessellationFactor);
	void SetUniformInsideTessFactor(uint32_t _tessellationFactor);
	void SetUniformTextureWidthHeight(uint32_t texWidth, uint32_t texHeight);
	void SetUniformTextureWidthHeightFluid(uint32_t texWidth, uint32_t texHeight);
	void SetUniformTextureWidthFluid(uint32_t texWidth);
	void SetUniformTextureHeightFluid(uint32_t texHeight);
	void SetUniformBlurRadius(uint32_t _blurRadius);
	void SetUniformMode(uint32_t _mode);

	float GetUniformHeightScale();
	float GetUniformWaveParticleSpeedScale();
	float GetUniformFlowSpeed();
	float GetUniformDxScale();
	float GetUniformDzScale();
	float GetUniformTimeScale();
	float GetUniformFoamScale();

	float GetUniformTimeStepFluid();
	float GetUniformFluidCellSize();
	float GetUniformFluidDissipation();
	float GetUniformVorticityScale();
	float GetUniformSplatDirU();
	float GetUniformSplatDirV();
	float GetUniformSplatScale();
	float GetUniformSplatDensityU();
	float GetUniformSplatDensityV();
	float GetUniformSplatDensityRadius();
	float GetUniformSplatDensityScale();

	float GetUniformBrushScale();
	float GetUniformBrushStrength();
	float GetUniformBrushOffsetU();
	float GetUniformBrushOffsetV();

	float GetUniformObstacleScale();
	float GetUniformObstacleThresholdFluid();
	float GetUniformObstacleThresholdWave();

	uint32_t GetUniformEdgeTessFactor();
	uint32_t GetUniformInsideTessFactor();
	uint32_t GetUniformTextureWidthFluid();
	uint32_t GetUniformTextureHeightFluid();
	uint32_t GetUniformBlurRadius();
	uint32_t GetUniformMode();

	float GetUniformLightHight();
	float GetUniformExtinctcoeff();
	float GetShiness();
	float GetfScale();
	float GetFpow();
	float GetBias();
	float GetFoamScale();

	void SetUniformLightHight(float v);
	void SetUniformExtinctcoeff(float v);
	void SetShiness(float v);
	void SetFScale(float v);
	void SetFPow(float v);
	void SetBias(float v);
	void SetFoamScale(float v);

	bool LoadScene();
	bool InitScene(ID3D12Device* device);
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();
	void Release();

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
