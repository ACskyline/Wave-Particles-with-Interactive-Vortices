#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Frame.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

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

	// Graphics Pipeline
	bool CreateGraphicsPipeline(
		ID3D12Device* device,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		Shader* vertexShader, 
		Shader* hullShader, 
		Shader* domainShader, 
		Shader* geometryShader, 
		Shader* pixelShader, 
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);

	void RecordGraphicsPipeline(
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		Frame* pFrame,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	ID3D12PipelineState* GetGraphicsPSO();

	// Post Process Pipeline
	bool CreatePostProcessPipeline(
		ID3D12Device* device,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		Shader* vertexShader, 
		Shader* hullShader, 
		Shader* domainShader, 
		Shader* geometryShader, 
		Shader* pixelShader, 
		const vector<Texture*>& textures,
		const vector<RenderTexture*>& renderTextures);

	void RecordPostProcessPipeline(
		CD3DX12_CPU_DESCRIPTOR_HANDLE renderTarget,
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencil,
		ID3D12GraphicsCommandList* commandList,
		Frame* pFrame,
		D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh

	ID3D12PipelineState* GetPostProcessPSO();

private:

	ID3D12Resource* depthStencilBuffer;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	ID3D12DescriptorHeap* dsvDescriptorHeap;

	ID3D12Resource* renderTargetBuffers[FrameBufferCount];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[FrameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;

	//graphics pipeline
	ID3D12PipelineState* graphicsPSO;
	ID3D12RootSignature* graphicsRootSignature;
	ID3D12DescriptorHeap* graphicsDescriptorHeap;
	ID3D12DescriptorHeap* graphicsRtvDescriptorHeap;

	bool CreateGraphicsPSO(
		ID3D12Device* device,
		ID3D12PipelineState** pso,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		Shader* vertexShader,
		Shader* hullShader,
		Shader* domainShader,
		Shader* geometryShader,
		Shader* pixelShader);

	bool CreateGraphicsRootSignature(ID3D12Device* device, int descriptorNum);

	//post process pipeline
	ID3D12PipelineState* postProcessPSO;
	ID3D12RootSignature* postProcessRootSignature;
	ID3D12DescriptorHeap* postProcessDescriptorHeap;
	ID3D12DescriptorHeap* postProcessRtvDescriptorHeap;

	bool CreatePostProcessPSO(
		ID3D12Device* device,
		ID3D12PipelineState** pso,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTType,
		Shader* vertexShader,
		Shader* hullShader,
		Shader* domainShader,
		Shader* geometryShader,
		Shader* pixelShader);

	bool CreatePostProcessRootSignature(ID3D12Device* device, int descriptorNum);
};

