#pragma once

#include "Scene.h"

class Renderer
{
public:
	// scoped enum to avoid redefinition of Count
	enum class GraphicsStage { CreateObstacle, Obstacle, WaterSurface, Count };
	enum class WaveParticleStage { Default, Count };
	enum class PostProcessStage { Horizontal, Vertical, ObstacleHorizontal, ObstacleVertical, Count };
	enum class FluidStage { AdvectVelocity, AdvectDensity, ComputeDivergence, Jacobi, SplatVelocity, SplatDensity, SubtractGradient, Count };

	Renderer();
	~Renderer();

	static D3D12_DEPTH_STENCIL_DESC NoDepthTest();
	static D3D12_BLEND_DESC AdditiveBlend();
	static D3D12_BLEND_DESC NoBlend();

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int frameIndex);
	ID3D12Resource* GetRenderTargetBuffer(int frameIndex);

	bool CreateRenderer(ID3D12Device* device, IDXGISwapChain3* swapChain, float Width, float Height);
	bool CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height);
	bool CreateRenderTargetBuffer(ID3D12Device* device, IDXGISwapChain3* swapChain);

	void Release();

	void RecordBegin(int frameIndex, ID3D12GraphicsCommandList* commandList);
	void RecordEnd(int frameIndex, ID3D12GraphicsCommandList* commandList);

	bool BindTextureToDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, Texture* texture, int slot);
	bool BindRenderTextureToRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, RenderTexture* texture, int slot);
	bool CreateDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** descriptorHeap, int descriptorNum);
	bool CreateRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** rtvDescriptorHeap, int descriptorNum);
	bool CreatePSO(
		ID3D12Device* device,
		ID3D12PipelineState** pso,
		ID3D12RootSignature* rootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		D3D12_BLEND_DESC blendDesc,
		D3D12_DEPTH_STENCIL_DESC dsDesc,
		DXGI_FORMAT rtvFormat,
		int rtvCount,
		Shader* vertexShader,
		Shader* hullShader,
		Shader* domainShader,
		Shader* geometryShader,
		Shader* pixelShader);
	void RecordPipelineNoClear(
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pso,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		CD3DX12_CPU_DESCRIPTOR_HANDLE fallbackRTV,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh
	void RecordPipeline(
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pso,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		CD3DX12_CPU_DESCRIPTOR_HANDLE fallbackRTV,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
		XMFLOAT4 clearColor = XMFLOAT4(0, 0, 0, 0));//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh
	void Clear(
		ID3D12GraphicsCommandList* commandList,
		CD3DX12_CPU_DESCRIPTOR_HANDLE fallbackRTV,
		Frame* pFrame = nullptr,
		XMFLOAT4 clearColor = XMFLOAT4(0, 0, 0, 0));
	bool CreateHeapBindTexture(
		ID3D12Device* device,
		ID3D12DescriptorHeap** descriptorHeap,
		ID3D12DescriptorHeap** rtvDescriptorHeap,
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);
	bool CreateHeap(
		ID3D12Device* device,
		ID3D12DescriptorHeap** descriptorHeap,
		ID3D12DescriptorHeap** rtvDescriptorHeap,
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

	//dynamically bound rtv and srv, so we need one heap for each frame
	//also we have more than one draw call during one frame, so we need
	//one heap for each draw call
	ID3D12PipelineState* fluidJacobiPSO[FrameBufferCount][JacobiIteration];
	ID3D12RootSignature* fluidJacobiRootSignature[FrameBufferCount][JacobiIteration];
	ID3D12DescriptorHeap* fluidJacobiDescriptorHeap[FrameBufferCount][JacobiIteration];
	ID3D12DescriptorHeap* fluidJacobiRtvDescriptorHeap[FrameBufferCount][JacobiIteration];

private:

	ID3D12Resource* depthStencilBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	ID3D12DescriptorHeap* dsvDescriptorHeap;

	ID3D12Resource* renderTargetBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;

	//graphics pipeline - statically bound rtv and srv
	ID3D12PipelineState* graphicsPSO[static_cast<int>(GraphicsStage::Count)];
	ID3D12RootSignature* graphicsRootSignature[static_cast<int>(GraphicsStage::Count)];
	ID3D12DescriptorHeap* graphicsDescriptorHeap[static_cast<int>(GraphicsStage::Count)];
	ID3D12DescriptorHeap* graphicsRtvDescriptorHeap[static_cast<int>(GraphicsStage::Count)];

	//wave particle pipeline - statically bound rtv and srv
	ID3D12PipelineState* waveParticlePSO[static_cast<int>(WaveParticleStage::Count)];
	ID3D12RootSignature* waveParticleRootSignature[static_cast<int>(WaveParticleStage::Count)];
	ID3D12DescriptorHeap* waveParticleDescriptorHeap[static_cast<int>(WaveParticleStage::Count)];
	ID3D12DescriptorHeap* waveParticleRtvDescriptorHeap[static_cast<int>(WaveParticleStage::Count)];

	//post process pipeline - statically bound rtv and srv
	ID3D12PipelineState* postProcessPSO[static_cast<int>(PostProcessStage::Count)];
	ID3D12RootSignature* postProcessRootSignature[static_cast<int>(PostProcessStage::Count)];
	ID3D12DescriptorHeap* postProcessDescriptorHeap[static_cast<int>(PostProcessStage::Count)];
	ID3D12DescriptorHeap* postProcessRtvDescriptorHeap[static_cast<int>(PostProcessStage::Count)];

	//fluid pipeline - dynamically bound rtv and srv, so we need one heap for each frame
	ID3D12PipelineState* fluidPSO[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12RootSignature* fluidRootSignature[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12DescriptorHeap* fluidDescriptorHeap[FrameBufferCount][static_cast<int>(FluidStage::Count)];
	ID3D12DescriptorHeap* fluidRtvDescriptorHeap[FrameBufferCount][static_cast<int>(FluidStage::Count)];

};

