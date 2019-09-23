#pragma once

#include "Scene.h"

class Renderer
{
public:
	// scoped enum to avoid redefinition of Count
	enum class GraphicsStage { CreateObstacle, Obstacle, WaterSurface, Count };
	enum class WaveParticleStage { Default, Count };
	enum class PostProcessStage { Horizontal, Vertical, ObstacleHorizontal, ObstacleVertical, Count };
	enum class FluidStage { AdvectVelocity, AdvectDensity, ComputeDivergence, /*Jacobi,*/ SplatVelocity, SplatDensity, SubtractGradient, Count };//Jacobi needs special treatment

	Renderer();
	~Renderer();

	static D3D12_DEPTH_STENCIL_DESC NoDepthTest();
	static D3D12_BLEND_DESC AdditiveBlend();
	static D3D12_BLEND_DESC NoBlend();

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvHandle(int frameIndex);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int frameIndex);
	ID3D12Resource* GetRenderTargetBuffer(int frameIndex);
	ID3D12Resource* GetDepthStencilBuffer(int frameIndex);

	bool CreateRenderer(ID3D12Device* device, IDXGISwapChain3* swapChain, float Width, float Height);
	bool CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height);
	bool CreateRenderTargetBuffer(ID3D12Device* device, IDXGISwapChain3* swapChain);

	void Release();

	void RecordBegin(int frameIndex, ID3D12GraphicsCommandList* commandList);
	void RecordEnd(int frameIndex, ID3D12GraphicsCommandList* commandList);

	bool BindTextureToDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, Texture* texture, int slot);
	bool BindRenderTextureToRtvDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12DescriptorHeap* dsvDescriptorHeap, RenderTexture* texture, int slot);
	bool CreateDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** descriptorHeap, int descriptorNum);
	bool CreateRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** rtvDescriptorHeap, int descriptorNum);
	bool CreateDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** dsvDescriptorHeap, int descriptorNum);
	bool CreatePSO(
		ID3D12Device* device,
		ID3D12PipelineState** pso,
		ID3D12RootSignature* rootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		D3D12_BLEND_DESC blendDesc,
		D3D12_DEPTH_STENCIL_DESC dsDesc,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat,
		int rtvCount,
		Shader* vertexShader,
		Shader* hullShader,
		Shader* domainShader,
		Shader* geometryShader,
		Shader* pixelShader,
		const wstring& name);
	void RecordPipeline(
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pso,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		Frame* pFrame,
		Scene* pScene,
		bool clearColor = true,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f,
		D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	void RecordPipelineOverride(
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pso,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		vector<RenderTexture*>& renderTextureVecOverride,
		Frame* pFrame,
		Scene* pScene,
		bool clearColor = true,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f,
		D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	void RecordPipelineOverride(
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pso,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		Frame* pFrame,
		Scene* pScene,
		bool clearColor = true,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f,
		D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	void Clear(
		ID3D12GraphicsCommandList* commandList,
		Frame* pFrame,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f);
	void ClearOverride(
		ID3D12GraphicsCommandList* commandList,
		vector<RenderTexture*>& renderTextureVecOverride,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f);
	void ClearOverride(
		ID3D12GraphicsCommandList* commandList,
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		bool clearDepth = true,
		XMFLOAT4 clearColorValue = XMFLOAT4(0, 0, 0, 0),
		float clearDepthValue = 1.0f);
	bool CreateHeapBindTexture(
		ID3D12Device* device,
		ID3D12DescriptorHeap** descriptorHeap,
		ID3D12DescriptorHeap** rtvDescriptorHeap,
		ID3D12DescriptorHeap** dsvDescriptorHeap,
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);
	bool CreateHeap(
		ID3D12Device* device,
		ID3D12DescriptorHeap** descriptorHeap,
		ID3D12DescriptorHeap** rtvDescriptorHeap,
		ID3D12DescriptorHeap** dsvDescriptorHeap,
		int textureCount,
		int renderTextureCount);//for dynamically bound heaps

	// Graphics Pipeline
	bool CreateGraphicsRootSignature(
		ID3D12Device* device, 
		ID3D12RootSignature** rootSignature, 
		int descriptorNum);

	ID3D12PipelineState* GetGraphicsPSO(int index);
	ID3D12PipelineState** GetGraphicsPsoPtr(int index);
	ID3D12RootSignature* GetGraphicsRootSignature(int index);
	ID3D12RootSignature** GetGraphicsRootSignaturePtr(int index);
	ID3D12DescriptorHeap* GetGraphicsDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetGraphicsDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetGraphicsRtvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetGraphicsRtvDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetGraphicsDsvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetGraphicsDsvDescriptorHeapPtr(int index);

	// Wave Particle Pipeline
	bool CreateWaveParticleRootSignature(
		ID3D12Device* device,
		ID3D12RootSignature** rootSignature,
		int descriptorNum);

	ID3D12PipelineState* GetWaveParticlePSO(int index);
	ID3D12PipelineState** GetWaveParticlePsoPtr(int index);
	ID3D12RootSignature* GetWaveParticleRootSignature(int index);
	ID3D12RootSignature** GetWaveParticleRootSignaturePtr(int index);
	ID3D12DescriptorHeap* GetWaveParticleDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetWaveParticleDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetWaveParticleRtvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetWaveParticleRtvDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetWaveParticleDsvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetWaveParticleDsvDescriptorHeapPtr(int index);

	// Post Process Pipeline
	bool CreatePostProcessRootSignature(
		ID3D12Device* device,
		ID3D12RootSignature** rootSignature,
		int descriptorNum);

	ID3D12PipelineState* GetPostProcessPSO(int index);
	ID3D12PipelineState** GetPostProcessPsoPtr(int index);
	ID3D12RootSignature* GetPostProcessRootSignature(int index);
	ID3D12RootSignature** GetPostProcessRootSignaturePtr(int index);
	ID3D12DescriptorHeap* GetPostProcessDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetPostProcessDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetPostProcessRtvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetPostProcessRtvDescriptorHeapPtr(int index);
	ID3D12DescriptorHeap* GetPostProcessDsvDescriptorHeap(int index);
	ID3D12DescriptorHeap** GetPostProcessDsvDescriptorHeapPtr(int index);

	// Fluid Pipeline
	bool CreateFluidRootSignature(
		ID3D12Device* device,
		ID3D12RootSignature** rootSignature,
		int descriptorNum);

	ID3D12PipelineState* GetFluidPSO(int frame, int index);
	ID3D12PipelineState** GetFluidPsoPtr(int frame, int index);
	ID3D12RootSignature* GetFluidRootSignature(int frame, int index);
	ID3D12RootSignature** GetFluidRootSignaturePtr(int frame, int index);
	ID3D12DescriptorHeap* GetFluidDescriptorHeap(int frame, int index);
	ID3D12DescriptorHeap** GetFluidDescriptorHeapPtr(int frame, int index);
	ID3D12DescriptorHeap* GetFluidRtvDescriptorHeap(int frame, int index);
	ID3D12DescriptorHeap** GetFluidRtvDescriptorHeapPtr(int frame, int index);
	ID3D12DescriptorHeap* GetFluidDsvDescriptorHeap(int frame, int index);
	ID3D12DescriptorHeap** GetFluidDsvDescriptorHeapPtr(int frame, int index);

	//dynamically bound rtv and srv, we need one heap for each frame
	//also we have more than one draw call (stage) during one frame, so we need
	//one heap for each draw call (stage)
	// * ping pong multiple times per frame *
	ID3D12PipelineState* fluidJacobiPSO[FrameBufferCount][JacobiIteration];
	ID3D12RootSignature* fluidJacobiRootSignature[FrameBufferCount][JacobiIteration];
	ID3D12DescriptorHeap* fluidJacobiDescriptorHeap[FrameBufferCount][JacobiIteration];
	ID3D12DescriptorHeap* fluidJacobiRtvDescriptorHeap[FrameBufferCount][JacobiIteration];
	ID3D12DescriptorHeap* fluidJacobiDsvDescriptorHeap[FrameBufferCount][JacobiIteration];

private:

	bool BindRenderTextureToRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, RenderTexture* texture, int slot);
	bool BindRenderTextureToDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, RenderTexture* texture, int slot);

	ID3D12Resource* depthStencilBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* dsvDescriptorHeap;

	ID3D12Resource* renderTargetBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;

	//graphics pipeline - statically bound rtv and srv
	ID3D12PipelineState* graphicsPSO[static_cast<int>(GraphicsStage::Count)];
	ID3D12RootSignature* graphicsRootSignature[static_cast<int>(GraphicsStage::Count)];
	ID3D12DescriptorHeap* graphicsDescriptorHeap[static_cast<int>(GraphicsStage::Count)];
	ID3D12DescriptorHeap* graphicsRtvDescriptorHeap[static_cast<int>(GraphicsStage::Count)];
	ID3D12DescriptorHeap* graphicsDsvDescriptorHeap[static_cast<int>(GraphicsStage::Count)];

	//wave particle pipeline - statically bound rtv and srv
	ID3D12PipelineState* waveParticlePSO[static_cast<int>(WaveParticleStage::Count)];
	ID3D12RootSignature* waveParticleRootSignature[static_cast<int>(WaveParticleStage::Count)];
	ID3D12DescriptorHeap* waveParticleDescriptorHeap[static_cast<int>(WaveParticleStage::Count)];
	ID3D12DescriptorHeap* waveParticleRtvDescriptorHeap[static_cast<int>(WaveParticleStage::Count)];
	ID3D12DescriptorHeap* waveParticleDsvDescriptorHeap[static_cast<int>(WaveParticleStage::Count)];

	//post process pipeline - statically bound rtv and srv
	ID3D12PipelineState* postProcessPSO[static_cast<int>(PostProcessStage::Count)];
	ID3D12RootSignature* postProcessRootSignature[static_cast<int>(PostProcessStage::Count)];
	ID3D12DescriptorHeap* postProcessDescriptorHeap[static_cast<int>(PostProcessStage::Count)];
	ID3D12DescriptorHeap* postProcessRtvDescriptorHeap[static_cast<int>(PostProcessStage::Count)];
	ID3D12DescriptorHeap* postProcessDsvDescriptorHeap[static_cast<int>(PostProcessStage::Count)];

	//fluid pipeline - dynamically bound rtv and srv, so we need one heap for each frame

	//dynamically bound rtv and srv, we need one heap for each frame
	//also we have more than one draw call (stage) during one frame, so we need
	//one heap for each draw call (stage)
	// * ping pong 1 time per frame *

	ID3D12PipelineState* fluidPSO[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12RootSignature* fluidRootSignature[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12DescriptorHeap* fluidDescriptorHeap[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12DescriptorHeap* fluidRtvDescriptorHeap[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12DescriptorHeap* fluidDsvDescriptorHeap[FrameBufferCount][static_cast<int>(FluidStage::Count)];

};

