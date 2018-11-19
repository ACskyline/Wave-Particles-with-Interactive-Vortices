#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Scene.h"

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	ID3D12PipelineState* GetGraphicsPSO();
	ID3D12RootSignature* GetGraphicsRootSignature();
	ID3D12DescriptorHeap* GetGraphicsHeap();

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int frameIndex);
	ID3D12Resource* GetRenderTargetBuffer(int frameIndex);

	bool CreateRenderer(ID3D12Device* device, IDXGISwapChain3* swapChain, float Width, float Height);
	bool CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height);
	bool CreateRenderTargetBuffer(ID3D12Device* device, IDXGISwapChain3* swapChain);

	bool CreateGraphicsPipeline(ID3D12Device* device, Shader* vertexShader, Shader* pixelShader, Texture* texture);
	bool CreateGraphicsPipeline(ID3D12Device* device, Shader* vertexShader, Shader* hullShader, Shader* domainShader, Shader* pixelShader, Texture* texture);
	bool CreateGraphicsPSO(ID3D12Device* device, Shader* vertexShader, Shader* pixelShader);
	bool CreateGraphicsPSO(ID3D12Device* device, Shader* vertexShader, Shader* hullShader, Shader* domainShader, Shader* pixelShader);
	bool CreateGraphicsRootSignature(ID3D12Device* device);
	bool CreateGraphicsDescriptorHeap(ID3D12Device* device);
	bool BindTextureToGraphicsDescriptor(ID3D12Device* device, Texture* texture);

	void RecordBegin(int frameIndex, ID3D12GraphicsCommandList* commandList);
	void RecordEnd(int frameIndex, ID3D12GraphicsCommandList* commandList);
	void RecordGraphicsPipeline(int frameIndex, ID3D12GraphicsCommandList* commandList, Scene* pScene);
	void RecordGraphicsPipelinePatch(int frameIndex, ID3D12GraphicsCommandList* commandList, Scene* pScene);

	void Release();

private:
	ID3D12PipelineState* graphicsPSO;
	ID3D12RootSignature* graphicsRootSignature;
	ID3D12DescriptorHeap* graphicsDescriptorHeap;

	ID3D12Resource* depthStencilBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	ID3D12DescriptorHeap* dsvDescriptorHeap; 

	ID3D12Resource* renderTargetBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap; 
};

