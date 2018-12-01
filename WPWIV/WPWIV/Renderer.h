#pragma once

#include "Scene.h"

class Renderer
{
public:
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

	// Graphics Pipeline
	bool CreateGraphicsPipeline(
		ID3D12Device* device,
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);

	void RecordGraphicsPipeline(
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	ID3D12PipelineState* GetGraphicsPSO(int index);
	ID3D12PipelineState** GetGraphicsPsoPtr(int index);
	ID3D12RootSignature* GetGraphicsRootSignature();
	ID3D12DescriptorHeap* GetGraphicsDescriptorHeap();

	// Wave Particle Pipeline
	bool CreateWaveParticlePipeline(
		ID3D12Device* device,
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);

	void RecordWaveParticlePipeline(
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	ID3D12PipelineState* GetWaveParticlePSO(int index);
	ID3D12PipelineState** GetWaveParticlePsoPtr(int index);
	ID3D12RootSignature* GetWaveParticleRootSignature();
	ID3D12DescriptorHeap* GetWaveParticleDescriptorHeap();

	// Post Process Pipeline
	bool CreatePostProcessPipeline(
		ID3D12Device* device,
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);

	void RecordPostProcessPipeline(
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	void RecordPostProcessPipeline(
		const vector<RenderTexture*> &renderTextureVec,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* descriptorHeap,
		Frame* pFrame,
		Scene* pScene,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	ID3D12PipelineState* GetPostProcessPSO(int index);
	ID3D12PipelineState** GetPostProcessPsoPtr(int index);
	ID3D12RootSignature* GetPostProcessRootSignature();
	ID3D12DescriptorHeap* GetPostProcessDescriptorHeap();

private:

	ID3D12Resource* depthStencilBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	ID3D12DescriptorHeap* dsvDescriptorHeap;

	ID3D12Resource* renderTargetBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;

	//graphics pipeline
	ID3D12PipelineState* graphicsPSO[1];
	ID3D12RootSignature* graphicsRootSignature;
	ID3D12DescriptorHeap* graphicsDescriptorHeap;
	ID3D12DescriptorHeap* graphicsRtvDescriptorHeap;

	bool CreateGraphicsRootSignature(
		ID3D12Device* device, 
		ID3D12RootSignature** rootSignature, 
		int descriptorNum);

	//wave particle pipeline
	ID3D12PipelineState* waveParticlePSO[1];
	ID3D12RootSignature* waveParticleRootSignature;
	ID3D12DescriptorHeap* waveParticleDescriptorHeap;
	ID3D12DescriptorHeap* waveParticleRtvDescriptorHeap;

	bool CreateWaveParticleRootSignature(
		ID3D12Device* device,
		ID3D12RootSignature** rootSignature,
		int descriptorNum);

	//post process pipeline
	ID3D12PipelineState* postProcessPSO[2];
	ID3D12RootSignature* postProcessRootSignature;
	ID3D12DescriptorHeap* postProcessDescriptorHeap;
	ID3D12DescriptorHeap* postProcessRtvDescriptorHeap;

	bool CreatePostProcessRootSignature(
		ID3D12Device* device,
		ID3D12RootSignature** rootSignature,
		int descriptorNum);
};

