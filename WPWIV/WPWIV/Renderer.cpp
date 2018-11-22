#include "Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	Release();
}

bool Renderer::CreateGraphicsRootSignature(ID3D12Device* device, int descriptorNum)
{
	HRESULT hr;
	// create root signature
	
	// create a descriptor range (descriptor table) and fill it out
	// this is a range of descriptors inside a descriptor heap
	D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
	descriptorTableRanges[0].NumDescriptors = descriptorNum; // we only have one texture right now, so the range is only 1
	descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
	descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	// create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

	// create a root parameter for the root descriptor and fill it out
	D3D12_ROOT_PARAMETER  rootParameters[4]; // 3 root parameters
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptors[3]; // 2 of 3 are cbv

	rootCBVDescriptors[0].RegisterSpace = 0;
	rootCBVDescriptors[0].ShaderRegister = 0;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[0].Descriptor = rootCBVDescriptors[0]; // this is the root descriptor for this root parameter
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now

	rootCBVDescriptors[1].RegisterSpace = 0;
	rootCBVDescriptors[1].ShaderRegister = 1;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[1].Descriptor = rootCBVDescriptors[1]; // this is the root descriptor for this root parameter
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now

	rootCBVDescriptors[2].RegisterSpace = 0;
	rootCBVDescriptors[2].ShaderRegister = 2;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[2].Descriptor = rootCBVDescriptors[2]; // this is the root descriptor for this root parameter
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now

	// fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
	// buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
	rootParameters[3].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		1, // we have one static sampler
		&sampler, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&graphicsRootSignature));
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool Renderer::CreateGraphicsPSO(ID3D12Device* device, Shader* vertexShader, Shader* hullShader, Shader* domainShader, Shader* geometryShader, Shader* pixelShader)
{
	HRESULT hr;
	// create input layout

	// fill out an input layout description structure
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
	inputLayoutDesc.NumElements = sizeof(VertexInputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = VertexInputLayout;

	// create a pipeline state object (PSO)

	// In a real application, you will have many pso's. for each different shader
	// or different combinations of shaders, different blend states or different rasterizer states,
	// different topology types (point, line, triangle, patch), or a different number
	// of render targets you will need a pso

	// VS is the only required shader for a pso. You might be wondering when a case would be where
	// you only set the VS. It's possible that you have a pso that only outputs data with the stream
	// output, and not on a render target, which means you would not need anything after the stream
	// output.

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = MultiSampleCount;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.pRootSignature = graphicsRootSignature; // the root signature that describes the input data this pso needs
	if (vertexShader != nullptr) psoDesc.VS = vertexShader->GetShaderByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
	if (hullShader != nullptr) psoDesc.HS = hullShader->GetShaderByteCode();
	if (domainShader != nullptr) psoDesc.DS = domainShader->GetShaderByteCode();
	if (geometryShader != nullptr) psoDesc.GS = geometryShader->GetShaderByteCode();
	if (pixelShader != nullptr) psoDesc.PS = pixelShader->GetShaderByteCode(); // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;// D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

	// create the pso
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPSO));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Renderer::CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height)
{
	HRESULT hr;
	// Create the depth/stencil buffer

	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);
	if (FAILED(hr))
	{
		return false;
	}
	dsvDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsvHandle);

	return true;
}

bool Renderer::CreateGraphicsDescriptorHeap(ID3D12Device* device, int descriptorNum)
{
	HRESULT hr;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = descriptorNum;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&graphicsDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool Renderer::CreateRenderTargetBuffer(ID3D12Device* device, IDXGISwapChain3* swapChain)
{
	HRESULT hr;

	// -- Create the Back Buffers (render target views) Descriptor Heap -- //

	// describe an rtv descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameBufferCount; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}

	// get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	int rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
	// but we cannot literally use it like a c++ pointer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
	for (int i = 0; i < FrameBufferCount; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetBuffers[i]));
		if (FAILED(hr))
		{
			return false;
		}

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		device->CreateRenderTargetView(renderTargetBuffers[i], nullptr, rtvHandle);
		rtvHandles[i] = rtvHandle;

		// we increment the rtv handle by the rtv descriptor size we got above
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	return true;
}

bool Renderer::BindTextureToDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, Texture* texture, int slot)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	int srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandle.Offset(slot, srvDescriptorSize);
	device->CreateShaderResourceView(texture->GetTextureBuffer(), &texture->GetSrvDesc(), descriptorHandle);
	return true;
}

ID3D12PipelineState* Renderer::GetGraphicsPSO()
{
	return graphicsPSO;
}

ID3D12RootSignature* Renderer::GetGraphicsRootSignature()
{
	return graphicsRootSignature;
}

ID3D12DescriptorHeap* Renderer::GetGraphicsHeap()
{
	return graphicsDescriptorHeap;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetRtvHandle(int frameIndex)
{
	return rtvHandles[frameIndex];
}

ID3D12Resource* Renderer::GetRenderTargetBuffer(int frameIndex)
{
	return renderTargetBuffers[frameIndex];
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetDsvHandle()
{
	return dsvHandle;
}

bool Renderer::CreateRenderer(ID3D12Device* device, IDXGISwapChain3* swapChain, float Width, float Height)
{
	if (!CreateRenderTargetBuffer(device, swapChain))
	{
		printf("InitRenderTargetBuffer failed\n");
		return false;
	}

	if (!CreateDepthStencilBuffer(device, Width, Height))
	{
		printf("CreateDepthStencilBuffer failed\n");
		return false;
	}

	return true;
}

bool Renderer::CreateGraphicsPipeline(ID3D12Device* device, Shader* vertexShader, Shader* hullShader, Shader* domainShader, Shader* geometryShader, Shader* pixelShader, const vector<Texture*> textures)
{
	int texCount = textures.size();

	if (!CreateGraphicsRootSignature(device, texCount))
	{
		printf("CreateRootSignature failed\n");
		return false;
	}

	if (!CreateGraphicsPSO(device, vertexShader, hullShader, domainShader, geometryShader, pixelShader))
	{
		printf("CreatePSO failed\n");
		return false;
	}

	if (!CreateGraphicsDescriptorHeap(device, texCount))
	{
		printf("CreateMainDescriptorHeap failed\n");
		return false;
	}

	for (int i = 0; i < texCount; i++)
	{
		if (!BindTextureToDescriptorHeap(device, graphicsDescriptorHeap, textures[i], i))
		{
			printf("BindTextureToMainDescriptor failed\n");
			return false;
		}
	}
	return true;
}

void Renderer::Release()
{
	SAFE_RELEASE(graphicsPSO);
	SAFE_RELEASE(graphicsRootSignature);

	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(dsvDescriptorHeap);

	SAFE_RELEASE(rtvDescriptorHeap);
	for (int i = 0; i < FrameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargetBuffers[i]);
	};
}

void Renderer::RecordBegin(int frameIndex, ID3D12GraphicsCommandList* commandList)
{
	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTargetBuffer(frameIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void Renderer::RecordEnd(int frameIndex, ID3D12GraphicsCommandList* commandList)
{
	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
	// warning if present is called on the render target when it's not in the present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTargetBuffer(frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void Renderer::RecordGraphicsPipeline(int frameIndex, ID3D12GraphicsCommandList* commandList, Scene* pScene)
{
	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &GetRtvHandle(frameIndex), FALSE, &GetDsvHandle());

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(GetRtvHandle(frameIndex), clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(GetDsvHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(GetGraphicsRootSignature()); // set the root signature

	// set the descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { GetGraphicsHeap() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootConstantBufferView(1, pScene->pCamera->GetUniformBufferGpuAddress());
	commandList->SetGraphicsRootDescriptorTable(3, GetGraphicsHeap()->GetGPUDescriptorHandleForHeapStart());
	commandList->RSSetViewports(1, &pScene->pCamera->GetViewport()); // set the viewports
	commandList->RSSetScissorRects(1, &pScene->pCamera->GetScissorRect()); // set the scissor rects
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	commandList->SetGraphicsRootConstantBufferView(2, pScene->GetUniformBufferGpuAddress());
	int meshCount = pScene->pMeshVec.size();
	for (int i = 0; i < meshCount; i++)
	{
		commandList->IASetVertexBuffers(0, 1, &pScene->pMeshVec[i]->GetVertexBufferView());// &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&pScene->pMeshVec[i]->GetIndexBufferView());//&indexBufferView);
		commandList->SetGraphicsRootConstantBufferView(0, pScene->pMeshVec[i]->GetUniformBufferGpuAddress());//0 can be changed to frameIndex
		commandList->DrawIndexedInstanced(pScene->pMeshVec[i]->GetIndexCount(), 1, 0, 0, 0);
	}

}

void Renderer::RecordGraphicsPipelinePatch(int frameIndex, ID3D12GraphicsCommandList* commandList, Scene* pScene)
{
	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &GetRtvHandle(frameIndex), FALSE, &GetDsvHandle());

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(GetRtvHandle(frameIndex), clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(GetDsvHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(GetGraphicsRootSignature()); // set the root signature

	// set the descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { GetGraphicsHeap() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootConstantBufferView(1, pScene->pCamera->GetUniformBufferGpuAddress());
	commandList->SetGraphicsRootDescriptorTable(3, GetGraphicsHeap()->GetGPUDescriptorHandleForHeapStart());
	commandList->RSSetViewports(1, &pScene->pCamera->GetViewport()); // set the viewports
	commandList->RSSetScissorRects(1, &pScene->pCamera->GetScissorRect()); // set the scissor rects
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // set the primitive topology
	commandList->SetGraphicsRootConstantBufferView(2, pScene->GetUniformBufferGpuAddress());
	int meshCount = pScene->pMeshVec.size();
	for (int i = 0; i < meshCount; i++)
	{
		commandList->IASetVertexBuffers(0, 1, &pScene->pMeshVec[i]->GetVertexBufferView());// &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&pScene->pMeshVec[i]->GetIndexBufferView());//&indexBufferView);
		commandList->SetGraphicsRootConstantBufferView(0, pScene->pMeshVec[i]->GetUniformBufferGpuAddress());//0 can be changed to frameIndex
		commandList->DrawIndexedInstanced(pScene->pMeshVec[i]->GetIndexCount(), 1, 0, 0, 0);
	}

}