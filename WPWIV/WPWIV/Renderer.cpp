#include "Renderer.h"

Renderer::Renderer()
{
	for (int i = 0; i < FrameBufferCount; i++)
	{
		for (int j = 0; j < static_cast<int>(FluidStage::Count); j++)
		{
			fluidPSO[i][j] = nullptr;
			fluidRootSignature[i][j] = nullptr;
			fluidDescriptorHeap[i][j] = nullptr;
			fluidRtvDescriptorHeap[i][j] = nullptr;
		}
	}
}

Renderer::~Renderer()
{
	Release();
}

D3D12_DEPTH_STENCIL_DESC Renderer::NoDepthTest()
{
	D3D12_DEPTH_STENCIL_DESC result = {};
	//ZeroMemory(&result, sizeof(D3D12_DEPTH_STENCIL_DESC));
	result.DepthEnable = FALSE;
	result.StencilEnable = FALSE;
	return result;
} 	

D3D12_BLEND_DESC Renderer::AdditiveBlend()
{
	D3D12_BLEND_DESC result = {};
	ZeroMemory(&result, sizeof(D3D12_BLEND_DESC));
	result.AlphaToCoverageEnable = FALSE;
	result.IndependentBlendEnable = FALSE;
	result.RenderTarget[0].BlendEnable = TRUE;
	result.RenderTarget[0].LogicOpEnable = FALSE;
	result.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	result.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	result.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	result.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	result.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	result.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	result.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_CLEAR;
	result.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return result;
}

D3D12_BLEND_DESC Renderer::NoBlend()
{
	D3D12_BLEND_DESC result = {};
	result.AlphaToCoverageEnable = FALSE;
	result.IndependentBlendEnable = FALSE;
	result.RenderTarget[0].BlendEnable = FALSE;
	//result.RenderTarget[0].LogicOpEnable = FALSE;
	//result.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	//result.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	//result.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	//result.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	//result.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	//result.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//result.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_CLEAR;
	result.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return result;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetRtvHandle(int frameIndex)
{
	return rtvHandles[frameIndex];
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetDsvHandle(int frameIndex)
{
	return dsvHandles[frameIndex];
}

ID3D12Resource* Renderer::GetRenderTargetBuffer(int frameIndex)
{
	return renderTargetBuffers[frameIndex];
}

ID3D12Resource* Renderer::GetDepthStencilBuffer(int frameIndex)
{
	return depthStencilBuffers[frameIndex];
}

bool Renderer::CreateRenderer(ID3D12Device* device, IDXGISwapChain3* swapChain, float Width, float Height)
{
	if (!CreateRenderTargetBuffer(device, swapChain))
	{
		printf("CreateRenderTargetBuffer failed\n");
		return false;
	}

	if (!CreateDepthStencilBuffer(device, Width, Height))
	{
		printf("CreateDepthStencilBuffer failed\n");
		return false;
	}

	return true;
}

//one depth stencil buffer is enough
bool Renderer::CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height)
{
	HRESULT hr;
	// Create the depth/stencil buffer

	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = FrameBufferCount;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}
	dsvDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	int dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < FrameBufferCount; i++)
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&depthStencilBuffers[i])
		);

		if (FAILED(hr))
		{
			return false;
		}

		device->CreateDepthStencilView(depthStencilBuffers[i], &depthStencilDesc, dsvHandle);
		
		dsvHandles[i] = dsvHandle;

		// we increment the dsv handle by the dsv descriptor size we got above
		dsvHandle.Offset(1, dsvDescriptorSize);
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
	rtvDescriptorHeap->SetName(L"Render Target Resource Heap");

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

void Renderer::Release()
{
	// fluid pipeline
	for (int i = 0; i < FrameBufferCount; i++)
	{
		for (int j = 0; j < static_cast<int>(FluidStage::Count); j++)
		{
			SAFE_RELEASE(fluidPSO[i][j]);
			SAFE_RELEASE(fluidRootSignature[i][j]);
			SAFE_RELEASE(fluidDescriptorHeap[i][j]);
			SAFE_RELEASE(fluidRtvDescriptorHeap[i][j]);
			SAFE_RELEASE(fluidDsvDescriptorHeap[i][j]);
		}
	}

	for (int i = 0; i < FrameBufferCount; i++)
	{
		for (int j = 0; j < JacobiIteration; j++)
		{
			SAFE_RELEASE(fluidJacobiPSO[i][j]);
			SAFE_RELEASE(fluidJacobiRootSignature[i][j]);
			SAFE_RELEASE(fluidJacobiDescriptorHeap[i][j]);
			SAFE_RELEASE(fluidJacobiRtvDescriptorHeap[i][j]);
			SAFE_RELEASE(fluidJacobiDsvDescriptorHeap[i][j]);
		}
	}

	// graphics pipeline
	SAFE_RELEASE_ARRAY(graphicsPSO);
	SAFE_RELEASE_ARRAY(graphicsRootSignature);
	SAFE_RELEASE_ARRAY(graphicsDescriptorHeap);
	SAFE_RELEASE_ARRAY(graphicsRtvDescriptorHeap);
	SAFE_RELEASE_ARRAY(graphicsDsvDescriptorHeap);

	// post process pipeline
	SAFE_RELEASE_ARRAY(postProcessPSO);
	SAFE_RELEASE_ARRAY(postProcessRootSignature);
	SAFE_RELEASE_ARRAY(postProcessDescriptorHeap);
	SAFE_RELEASE_ARRAY(postProcessRtvDescriptorHeap);
	SAFE_RELEASE_ARRAY(postProcessDsvDescriptorHeap);

	// wave particle pipeline
	SAFE_RELEASE_ARRAY(waveParticlePSO);
	SAFE_RELEASE_ARRAY(waveParticleRootSignature);
	SAFE_RELEASE_ARRAY(waveParticleDescriptorHeap);
	SAFE_RELEASE_ARRAY(waveParticleRtvDescriptorHeap);
	SAFE_RELEASE_ARRAY(waveParticleDsvDescriptorHeap);

	// common resource
	SAFE_RELEASE(dsvDescriptorHeap);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE_ARRAY(depthStencilBuffers);
	SAFE_RELEASE_ARRAY(renderTargetBuffers);
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

bool Renderer::BindTextureToDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, Texture* texture, int slot)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	int srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandle.Offset(slot, srvDescriptorSize);
	device->CreateShaderResourceView(texture->GetTextureBuffer(), &texture->GetSrvDesc(), descriptorHandle);
	return true;
}

bool Renderer::BindRenderTextureToRtvDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12DescriptorHeap* dsvDescriptorHeap, RenderTexture* texture, int slot)
{
	if (!BindRenderTextureToRtvDescriptorHeap(device, rtvDescriptorHeap, texture, slot))
		return false;

	if (!BindRenderTextureToDsvDescriptorHeap(device, dsvDescriptorHeap, texture, slot))
		return false;

	return true;
}

bool Renderer::BindRenderTextureToRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, RenderTexture* texture, int slot)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	int rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorHandle.Offset(slot, rtvDescriptorSize);
	device->CreateRenderTargetView(texture->GetTextureBuffer(), &texture->GetRtvDesc(), descriptorHandle);
	texture->SetRtvHandle(descriptorHandle);
	return true;
}

bool Renderer::BindRenderTextureToDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap, RenderTexture* texture, int slot)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	int dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	descriptorHandle.Offset(slot, dsvDescriptorSize);
	device->CreateDepthStencilView(texture->GetDepthStencilBuffer(), &texture->GetDsvDesc(), descriptorHandle);
	texture->SetDsvHandle(descriptorHandle);
	return true;
}

bool Renderer::CreateDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** descriptorHeap, int descriptorNum)
{
	if (descriptorNum > 0)
	{
		HRESULT hr;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = descriptorNum;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(descriptorHeap));
		if (FAILED(hr))
		{
			return false;
		}
	}
	else
	{
		*descriptorHeap = nullptr;
	}
	return true;
}

bool Renderer::CreateRtvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** rtvDescriptorHeap, int descriptorNum)
{
	if (descriptorNum > 0)
	{
		HRESULT hr;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = descriptorNum;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvDescriptorHeap));
		if (FAILED(hr))
		{
			return false;
		}
	}
	else
	{
		*rtvDescriptorHeap = nullptr;
	}
	return true;
}

bool Renderer::CreateDsvDescriptorHeap(ID3D12Device* device, ID3D12DescriptorHeap** dsvDescriptorHeap, int descriptorNum)
{
	if (descriptorNum > 0)
	{
		HRESULT hr;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = descriptorNum;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(dsvDescriptorHeap));
		if (FAILED(hr))
		{
			return false;
		}
	}
	else
	{
		*dsvDescriptorHeap = nullptr;
	}
	return true;
}

bool Renderer::CreatePSO(
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
	const wstring& name)
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
	psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
	if (vertexShader != nullptr) psoDesc.VS = vertexShader->GetShaderByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
	if (hullShader != nullptr) psoDesc.HS = hullShader->GetShaderByteCode();
	if (domainShader != nullptr) psoDesc.DS = domainShader->GetShaderByteCode();
	if (geometryShader != nullptr) psoDesc.GS = geometryShader->GetShaderByteCode();
	if (pixelShader != nullptr) psoDesc.PS = pixelShader->GetShaderByteCode(); // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = primitiveTType;// D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	for (int i = 0; i < rtvCount; i++)
	{
		psoDesc.RTVFormats[i] = rtvFormat; // format of the render target
	}
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = blendDesc;// CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = rtvCount; // we are only binding one render target
	psoDesc.DepthStencilState = dsDesc;// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
	psoDesc.DSVFormat = dsvFormat;
	// create the pso
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pso));
	if (FAILED(hr))
	{
		CheckError(hr);
		return false;
	}

	(*pso)->SetName(name.c_str());

	return true;
}

void Renderer::RecordPipeline(
	ID3D12GraphicsCommandList* commandList,
	ID3D12PipelineState* pso,
	ID3D12RootSignature* rootSignature,
	ID3D12DescriptorHeap* descriptorHeap,
	Frame* pFrame,
	Scene* pScene,
	bool clearColor,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue,
	D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride)//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh
{
	vector<RenderTexture*>& renderTextureVec = pFrame->GetRenderTextureVec();
	int renderTextureCount = renderTextureVec.size();
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameRtvHandles;
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameDsvHandles;//!!!* only the first depth stencil buffer will be used *!!!//
	vector<D3D12_VIEWPORT> frameViewPorts;
	vector<D3D12_RECT> frameScissorRects;
	frameRtvHandles.resize(renderTextureCount);
	frameDsvHandles.resize(renderTextureCount);//!!!* only the first depth stencil buffer will be used *!!!//
	frameViewPorts.resize(renderTextureCount);
	frameScissorRects.resize(renderTextureCount);
	for (int i = 0; i < renderTextureCount; i++)
	{
		frameRtvHandles[i] = renderTextureVec[i]->GetRtvHandle();
		frameDsvHandles[i] = renderTextureVec[i]->GetDsvHandle();//!!!* only the first depth stencil buffer will be used *!!!//
		frameViewPorts[i] = renderTextureVec[i]->GetViewport();
		frameScissorRects[i] = renderTextureVec[i]->GetScissorRect();
	}

	commandList->SetPipelineState(pso);

	// set the render target for the output merger stage (the output of the pipeline)
	//!!!* only the first depth stencil buffer will be used *!!!//
	//the last parameter is the address of one dsv handle
	commandList->OMSetRenderTargets(renderTextureCount, frameRtvHandles.data(), FALSE, frameDsvHandles.data());

	// Clear the render target by using the ClearRenderTargetView command
	if (clearColor || clearDepth)
	{
		const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };
		for (int i = 0; i < renderTextureCount; i++)
		{
			// clear the render target buffer
			if(clearColor)
				commandList->ClearRenderTargetView(frameRtvHandles[i], clearColorValueV, 0, nullptr);
			// clear the depth/stencil buffer
			if(clearDepth)
				commandList->ClearDepthStencilView(frameDsvHandles[i], D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
		}
	}

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

	// set the descriptor heap
	if (descriptorHeap != nullptr)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		commandList->SetGraphicsRootDescriptorTable(UNIFORM_SLOT::TABLE, descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::CAMERA, pFrame->GetCameraVec()[0]->GetUniformBufferGpuAddress());
	commandList->RSSetViewports(renderTextureCount, frameViewPorts.data()); // set the viewports
	commandList->RSSetScissorRects(renderTextureCount, frameScissorRects.data()); // set the scissor rects
	if (primitiveTypeOverride != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(primitiveTypeOverride); // set the primitive topology
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::FRAME, pFrame->GetUniformBufferGpuAddress());
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::SCENE, pScene->GetUniformBufferGpuAddress());
	int meshCount = pFrame->GetMeshVec().size();
	for (int i = 0; i < meshCount; i++)
	{
		if (primitiveTypeOverride == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(pFrame->GetMeshVec()[i]->GetPrimitiveType());
		commandList->IASetVertexBuffers(0, 1, &pFrame->GetMeshVec()[i]->GetVertexBufferView());// &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&pFrame->GetMeshVec()[i]->GetIndexBufferView());//&indexBufferView);
		commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::OBJECT, pFrame->GetMeshVec()[i]->GetUniformBufferGpuAddress());//0 can be changed to frameIndex
		commandList->DrawIndexedInstanced(pFrame->GetMeshVec()[i]->GetIndexCount(), 1, 0, 0, 0);
	}
}

void Renderer::RecordPipelineOverride(
	ID3D12GraphicsCommandList* commandList,
	ID3D12PipelineState* pso,
	ID3D12RootSignature* rootSignature,
	ID3D12DescriptorHeap* descriptorHeap,
	vector<RenderTexture*>& renderTextureVecOverride,
	Frame* pFrame,
	Scene* pScene,
	bool clearColor,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue,
	D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride)//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh
{
	int renderTextureCount = renderTextureVecOverride.size();
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameRtvHandles;
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameDsvHandles;//!!!* only the first depth stencil buffer will be used *!!!//
	vector<D3D12_VIEWPORT> frameViewPorts;
	vector<D3D12_RECT> frameScissorRects;
	frameRtvHandles.resize(renderTextureCount);
	frameDsvHandles.resize(renderTextureCount);//!!!* only the first depth stencil buffer will be used *!!!//
	frameViewPorts.resize(renderTextureCount);
	frameScissorRects.resize(renderTextureCount);
	for (int i = 0; i < renderTextureCount; i++)
	{
		frameRtvHandles[i] = renderTextureVecOverride[i]->GetRtvHandle();
		frameDsvHandles[i] = renderTextureVecOverride[i]->GetDsvHandle();//!!!* only the first depth stencil buffer will be used *!!!//
		frameViewPorts[i] = renderTextureVecOverride[i]->GetViewport();
		frameScissorRects[i] = renderTextureVecOverride[i]->GetScissorRect();
	}

	commandList->SetPipelineState(pso);

	// set the render target for the output merger stage (the output of the pipeline)
	//!!!* only the first depth stencil buffer will be used *!!!//
	//the last parameter is the address of one dsv handle
	commandList->OMSetRenderTargets(renderTextureCount, frameRtvHandles.data(), FALSE, frameDsvHandles.data());

	// Clear the render target by using the ClearRenderTargetView command
	if (clearColor || clearDepth)
	{
		const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };
		for (int i = 0; i < renderTextureCount; i++)
		{
			// clear the render target buffer
			if (clearColor)
				commandList->ClearRenderTargetView(frameRtvHandles[i], clearColorValueV, 0, nullptr);
			// clear the depth/stencil buffer
			if (clearDepth)
				commandList->ClearDepthStencilView(frameDsvHandles[i], D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
		}
	}

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

	// set the descriptor heap
	if (descriptorHeap != nullptr)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		commandList->SetGraphicsRootDescriptorTable(UNIFORM_SLOT::TABLE, descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::CAMERA, pFrame->GetCameraVec()[0]->GetUniformBufferGpuAddress());
	commandList->RSSetViewports(renderTextureCount, frameViewPorts.data()); // set the viewports
	commandList->RSSetScissorRects(renderTextureCount, frameScissorRects.data()); // set the scissor rects
	if (primitiveTypeOverride != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(primitiveTypeOverride); // set the primitive topology
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::FRAME, pFrame->GetUniformBufferGpuAddress());
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::SCENE, pScene->GetUniformBufferGpuAddress());
	int meshCount = pFrame->GetMeshVec().size();
	for (int i = 0; i < meshCount; i++)
	{
		if (primitiveTypeOverride == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(pFrame->GetMeshVec()[i]->GetPrimitiveType());
		commandList->IASetVertexBuffers(0, 1, &pFrame->GetMeshVec()[i]->GetVertexBufferView());// &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&pFrame->GetMeshVec()[i]->GetIndexBufferView());//&indexBufferView);
		commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::OBJECT, pFrame->GetMeshVec()[i]->GetUniformBufferGpuAddress());//0 can be changed to frameIndex
		commandList->DrawIndexedInstanced(pFrame->GetMeshVec()[i]->GetIndexCount(), 1, 0, 0, 0);
	}
}

void Renderer::RecordPipelineOverride(
	ID3D12GraphicsCommandList* commandList,
	ID3D12PipelineState* pso,
	ID3D12RootSignature* rootSignature,
	ID3D12DescriptorHeap* descriptorHeap,
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle,
	Frame* pFrame,
	Scene* pScene,
	bool clearColor,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue,
	D3D_PRIMITIVE_TOPOLOGY primitiveTypeOverride)//pass in D3D_PRIMITIVE_TOPOLOGY_UNDEFINED to use primitive type of each mesh
{
	commandList->SetPipelineState(pso);

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Clear the render target by using the ClearRenderTargetView command
	if (clearColor)
	{
		const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };
		// clear the render target buffer
		commandList->ClearRenderTargetView(rtvHandle, clearColorValueV, 0, nullptr);
	}

	if (clearDepth)
	{
		// clear the depth/stencil buffer
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
	}

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

	// set the descriptor heap
	if (descriptorHeap != nullptr)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		commandList->SetGraphicsRootDescriptorTable(UNIFORM_SLOT::TABLE, descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::CAMERA, pFrame->GetCameraVec()[0]->GetUniformBufferGpuAddress());
	commandList->RSSetViewports(1, &pFrame->GetCameraVec()[0]->GetViewport()); // set the viewports
	commandList->RSSetScissorRects(1, &pFrame->GetCameraVec()[0]->GetScissorRect()); // set the scissor rects
	if (primitiveTypeOverride != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(primitiveTypeOverride); // set the primitive topology
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::FRAME, pFrame->GetUniformBufferGpuAddress());
	commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::SCENE, pScene->GetUniformBufferGpuAddress());
	int meshCount = pFrame->GetMeshVec().size();
	for (int i = 0; i < meshCount; i++)
	{
		if (primitiveTypeOverride == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) commandList->IASetPrimitiveTopology(pFrame->GetMeshVec()[i]->GetPrimitiveType());
		commandList->IASetVertexBuffers(0, 1, &pFrame->GetMeshVec()[i]->GetVertexBufferView());// &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&pFrame->GetMeshVec()[i]->GetIndexBufferView());//&indexBufferView);
		commandList->SetGraphicsRootConstantBufferView(UNIFORM_SLOT::OBJECT, pFrame->GetMeshVec()[i]->GetUniformBufferGpuAddress());//0 can be changed to frameIndex
		commandList->DrawIndexedInstanced(pFrame->GetMeshVec()[i]->GetIndexCount(), 1, 0, 0, 0);
	}
}

void Renderer::Clear(
	ID3D12GraphicsCommandList* commandList,
	Frame* pFrame,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue)
{
	vector<RenderTexture*>& renderTextureVec = pFrame->GetRenderTextureVec();
	int renderTextureCount = renderTextureVec.size();
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameRtvHandles;
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameDsvHandles;
	frameRtvHandles.resize(renderTextureCount);
	frameDsvHandles.resize(renderTextureCount);
	for (int i = 0; i < renderTextureCount; i++)
	{
		frameRtvHandles[i] = renderTextureVec[i]->GetRtvHandle();
		frameDsvHandles[i] = renderTextureVec[i]->GetDsvHandle();
	}

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(renderTextureCount, frameRtvHandles.data(), FALSE, frameDsvHandles.data());

	// Clear the render target by using the ClearRenderTargetView command

	const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };
	for (int i = 0; i < renderTextureCount; i++)
	{
		// clear the render target buffer
		commandList->ClearRenderTargetView(frameRtvHandles[i], clearColorValueV, 0, nullptr);
		// clear the depth/stencil buffer
		if(clearDepth)
			commandList->ClearDepthStencilView(frameDsvHandles[i], D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
	}
}

void Renderer::ClearOverride(
	ID3D12GraphicsCommandList* commandList,
	vector<RenderTexture*>& renderTextureVecOverride,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue)
{
	int renderTextureCount = renderTextureVecOverride.size();
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameRtvHandles;
	vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> frameDsvHandles;
	frameRtvHandles.resize(renderTextureCount);
	frameDsvHandles.resize(renderTextureCount);
	for (int i = 0; i < renderTextureCount; i++)
	{
		frameRtvHandles[i] = renderTextureVecOverride[i]->GetRtvHandle();
		frameDsvHandles[i] = renderTextureVecOverride[i]->GetDsvHandle();
	}

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(renderTextureCount, frameRtvHandles.data(), FALSE, frameDsvHandles.data());

	// Clear the render target by using the ClearRenderTargetView command

	const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };
	for (int i = 0; i < renderTextureCount; i++)
	{
		// clear the render target buffer
		commandList->ClearRenderTargetView(frameRtvHandles[i], clearColorValueV, 0, nullptr);
		// clear the depth/stencil buffer
		if(clearDepth)
			commandList->ClearDepthStencilView(frameDsvHandles[i], D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
	}
}

void Renderer::ClearOverride(
	ID3D12GraphicsCommandList* commandList,
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle,
	bool clearDepth,
	XMFLOAT4 clearColorValue,
	float clearDepthValue)
{

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Clear the render target by using the ClearRenderTargetView command

	const float clearColorValueV[] = { clearColorValue.x, clearColorValue.y, clearColorValue.z, clearColorValue.w };

	// clear the render target buffer
	commandList->ClearRenderTargetView(rtvHandle, clearColorValueV, 0, nullptr);
	// clear the depth/stencil buffer
	if(clearDepth)
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, clearDepthValue, 0, 0, nullptr);
}

bool Renderer::CreateHeapBindTexture(
	ID3D12Device* device,
	ID3D12DescriptorHeap** descriptorHeap,
	ID3D12DescriptorHeap** rtvDescriptorHeap,
	ID3D12DescriptorHeap** dsvDescriptorHeap,
	const vector<Texture*>& textures,
	const vector<RenderTexture*>& renderTextures)
{
	int texCount = textures.size();
	int renderTexCount = renderTextures.size();

	if (!CreateDescriptorHeap(device, descriptorHeap, texCount))
	{
		printf("CreateDescriptorHeap failed\n");
		return false;
	}

	for (int i = 0; i < texCount; i++)
	{
		if (!BindTextureToDescriptorHeap(device, *descriptorHeap, textures[i], i))
		{
			printf("BindTextureToDescriptorHeap failed\n");
			return false;
		}
	}

	if (!CreateRtvDescriptorHeap(device, rtvDescriptorHeap, renderTexCount))
	{
		printf("CreateRtvDescriptorHeap failed\n");
		return false;
	}

	if (!CreateDsvDescriptorHeap(device, dsvDescriptorHeap, renderTexCount))
	{
		printf("CreateDsvDescriptorHeap failed\n");
		return false;
	}

	for (int i = 0; i < renderTexCount; i++)
	{
		if (!BindRenderTextureToRtvDescriptorHeap(device, *rtvDescriptorHeap, renderTextures[i], i))
		{
			printf("BindRenderTextureToRtvDescriptorHeap failed\n");
			return false;
		}
		
		if (!BindRenderTextureToDsvDescriptorHeap(device, *dsvDescriptorHeap, renderTextures[i], i))
		{
			printf("BindRenderTextureToDsvDescriptorHeap failed\n");
			return false;
		}
	}

	return true;
}

bool Renderer::CreateHeap(
	ID3D12Device* device,
	ID3D12DescriptorHeap** descriptorHeap,
	ID3D12DescriptorHeap** rtvDescriptorHeap,
	ID3D12DescriptorHeap** dsvDescriptorHeap,
	int textureCount,
	int renderTextureCount)
{
	if (!CreateDescriptorHeap(device, descriptorHeap, textureCount))
	{
		printf("CreateDescriptorHeap failed\n");
		return false;
	}

	if (!CreateRtvDescriptorHeap(device, rtvDescriptorHeap, renderTextureCount))
	{
		printf("CreateRtvDescriptorHeap failed\n");
		return false;
	}

	if (!CreateDsvDescriptorHeap(device, dsvDescriptorHeap, renderTextureCount))
	{
		printf("CreateDsvDescriptorHeap failed\n");
		return false;
	}

	return true;
}

////////////////////////////////////////////
////////// Wave Particle Pipeline //////////
////////////////////////////////////////////

bool Renderer::CreateWaveParticleRootSignature(
	ID3D12Device* device,
	ID3D12RootSignature** rootSignature,
	int descriptorNum)
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
	D3D12_ROOT_PARAMETER  rootParameters[UNIFORM_SLOT::COUNT]; // 3 root parameters
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptors[UNIFORM_SLOT::COUNT - 1]; // 2 of 3 are cbv
	UINT rootParameterCount = 0;

	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].ShaderRegister = UNIFORM_SLOT::OBJECT;
	rootParameters[UNIFORM_SLOT::OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::OBJECT].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::OBJECT]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].ShaderRegister = UNIFORM_SLOT::CAMERA;
	rootParameters[UNIFORM_SLOT::CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::CAMERA].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::CAMERA]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::FRAME].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::FRAME].ShaderRegister = UNIFORM_SLOT::FRAME;
	rootParameters[UNIFORM_SLOT::FRAME].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::FRAME].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::FRAME]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::FRAME].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::SCENE].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::SCENE].ShaderRegister = UNIFORM_SLOT::SCENE;
	rootParameters[UNIFORM_SLOT::SCENE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::SCENE].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::SCENE]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::SCENE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	if (descriptorNum > 0)
	{
		// fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
		// buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
		rootParameters[UNIFORM_SLOT::TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		rootParameters[UNIFORM_SLOT::TABLE].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		rootParameters[UNIFORM_SLOT::TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
		rootParameterCount++;
	}

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
	rootSignatureDesc.Init(rootParameterCount, // we have 2 root parameters
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
		CheckError(hr, errorBuff);
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature));
	
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

ID3D12PipelineState* Renderer::GetWaveParticlePSO(int index)
{
	return waveParticlePSO[index];
}

ID3D12PipelineState** Renderer::GetWaveParticlePsoPtr(int index)
{
	return &waveParticlePSO[index];
}

ID3D12RootSignature* Renderer::GetWaveParticleRootSignature(int index)
{
	return waveParticleRootSignature[index];
}

ID3D12RootSignature** Renderer::GetWaveParticleRootSignaturePtr(int index)
{
	return &waveParticleRootSignature[index];
}

ID3D12DescriptorHeap* Renderer::GetWaveParticleDescriptorHeap(int index)
{
	return  waveParticleDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetWaveParticleDescriptorHeapPtr(int index)
{
	return  &waveParticleDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetWaveParticleRtvDescriptorHeap(int index)
{
	return  waveParticleRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetWaveParticleRtvDescriptorHeapPtr(int index)
{
	return  &waveParticleRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetWaveParticleDsvDescriptorHeap(int index)
{
	return  waveParticleDsvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetWaveParticleDsvDescriptorHeapPtr(int index)
{
	return  &waveParticleDsvDescriptorHeap[index];
}

///////////////////////////////////////////
////////// Post Process Pipeline //////////
///////////////////////////////////////////

bool Renderer::CreatePostProcessRootSignature(
	ID3D12Device* device,
	ID3D12RootSignature** rootSignature,
	int descriptorNum)
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
	D3D12_ROOT_PARAMETER  rootParameters[UNIFORM_SLOT::COUNT]; // 3 root parameters
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptors[UNIFORM_SLOT::COUNT - 1]; // 2 of 3 are cbv
	UINT rootParameterCount = 0;

	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].ShaderRegister = UNIFORM_SLOT::OBJECT;
	rootParameters[UNIFORM_SLOT::OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::OBJECT].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::OBJECT]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].ShaderRegister = UNIFORM_SLOT::CAMERA;
	rootParameters[UNIFORM_SLOT::CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::CAMERA].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::CAMERA]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::FRAME].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::FRAME].ShaderRegister = UNIFORM_SLOT::FRAME;
	rootParameters[UNIFORM_SLOT::FRAME].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::FRAME].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::FRAME]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::FRAME].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::SCENE].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::SCENE].ShaderRegister = UNIFORM_SLOT::SCENE;
	rootParameters[UNIFORM_SLOT::SCENE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::SCENE].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::SCENE]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::SCENE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	if (descriptorNum > 0)
	{
		// fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
		// buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
		rootParameters[UNIFORM_SLOT::TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		rootParameters[UNIFORM_SLOT::TABLE].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		rootParameters[UNIFORM_SLOT::TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
		rootParameterCount++;
	}

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	rootSignatureDesc.Init(rootParameterCount, // we have 2 root parameters
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
		CheckError(hr, errorBuff);
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature));
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

ID3D12PipelineState* Renderer::GetPostProcessPSO(int index)
{
	return postProcessPSO[index];
}

ID3D12PipelineState** Renderer::GetPostProcessPsoPtr(int index)
{
	return &postProcessPSO[index];
}

ID3D12RootSignature* Renderer::GetPostProcessRootSignature(int index)
{
	return postProcessRootSignature[index];
}

ID3D12RootSignature** Renderer::GetPostProcessRootSignaturePtr(int index)
{
	return &postProcessRootSignature[index];
}

ID3D12DescriptorHeap* Renderer::GetPostProcessDescriptorHeap(int index)
{
	return  postProcessDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetPostProcessDescriptorHeapPtr(int index)
{
	return  &postProcessDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetPostProcessRtvDescriptorHeap(int index)
{
	return  postProcessRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetPostProcessRtvDescriptorHeapPtr(int index)
{
	return  &postProcessRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetPostProcessDsvDescriptorHeap(int index)
{
	return  postProcessDsvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetPostProcessDsvDescriptorHeapPtr(int index)
{
	return  &postProcessDsvDescriptorHeap[index];
}

///////////////////////////////////////////
//////////// Graphics Pipeline ////////////
///////////////////////////////////////////

bool Renderer::CreateGraphicsRootSignature(
	ID3D12Device* device,
	ID3D12RootSignature** rootSignature,
	int descriptorNum)
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
	D3D12_ROOT_PARAMETER  rootParameters[UNIFORM_SLOT::COUNT]; // 3 root parameters
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptors[UNIFORM_SLOT::COUNT - 1]; // 2 of 3 are cbv
	UINT rootParameterCount = 0;

	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].ShaderRegister = UNIFORM_SLOT::OBJECT;
	rootParameters[UNIFORM_SLOT::OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::OBJECT].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::OBJECT]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].ShaderRegister = UNIFORM_SLOT::CAMERA;
	rootParameters[UNIFORM_SLOT::CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::CAMERA].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::CAMERA]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::FRAME].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::FRAME].ShaderRegister = UNIFORM_SLOT::FRAME;
	rootParameters[UNIFORM_SLOT::FRAME].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::FRAME].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::FRAME]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::FRAME].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::SCENE].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::SCENE].ShaderRegister = UNIFORM_SLOT::SCENE;
	rootParameters[UNIFORM_SLOT::SCENE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::SCENE].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::SCENE]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::SCENE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	if (descriptorNum > 0)
	{
		// fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
		// buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
		rootParameters[UNIFORM_SLOT::TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		rootParameters[UNIFORM_SLOT::TABLE].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		rootParameters[UNIFORM_SLOT::TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
		rootParameterCount++;
	}

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC samplerWrap = {};
	samplerWrap.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.MipLODBias = 0;
	samplerWrap.MaxAnisotropy = 0;
	samplerWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerWrap.MinLOD = 0.0f;
	samplerWrap.MaxLOD = D3D12_FLOAT32_MAX;
	samplerWrap.ShaderRegister = 0;
	samplerWrap.RegisterSpace = 0;
	samplerWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC samplerClamp = {};
	samplerClamp.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerClamp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerClamp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerClamp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerClamp.MipLODBias = 0;
	samplerClamp.MaxAnisotropy = 0;
	samplerClamp.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerClamp.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerClamp.MinLOD = 0.0f;
	samplerClamp.MaxLOD = D3D12_FLOAT32_MAX;
	samplerClamp.ShaderRegister = 1;
	samplerClamp.RegisterSpace = 0;
	samplerClamp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC samplers[] = { samplerWrap, samplerClamp };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(rootParameterCount, // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		_countof(samplers), // we have one static sampler
		samplers, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature));
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

ID3D12PipelineState* Renderer::GetGraphicsPSO(int index)
{
	return graphicsPSO[index];
}

ID3D12PipelineState** Renderer::GetGraphicsPsoPtr(int index)
{
	return &graphicsPSO[index];
}

ID3D12RootSignature* Renderer::GetGraphicsRootSignature(int index)
{
	return graphicsRootSignature[index];
}

ID3D12RootSignature** Renderer::GetGraphicsRootSignaturePtr(int index)
{
	return &graphicsRootSignature[index];
}

ID3D12DescriptorHeap* Renderer::GetGraphicsDescriptorHeap(int index)
{
	return  graphicsDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetGraphicsDescriptorHeapPtr(int index)
{
	return  &graphicsDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetGraphicsRtvDescriptorHeap(int index)
{
	return  graphicsRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetGraphicsRtvDescriptorHeapPtr(int index)
{
	return  &graphicsRtvDescriptorHeap[index];
}

ID3D12DescriptorHeap* Renderer::GetGraphicsDsvDescriptorHeap(int index)
{
	return  graphicsDsvDescriptorHeap[index];
}

ID3D12DescriptorHeap** Renderer::GetGraphicsDsvDescriptorHeapPtr(int index)
{
	return  &graphicsDsvDescriptorHeap[index];
}

////////////////////////////////////////////
////////////// Fluid Pipeline //////////////
////////////////////////////////////////////

bool Renderer::CreateFluidRootSignature(
	ID3D12Device* device,
	ID3D12RootSignature** rootSignature,
	int descriptorNum)
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
	D3D12_ROOT_PARAMETER  rootParameters[UNIFORM_SLOT::COUNT]; // 3 root parameters
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptors[UNIFORM_SLOT::COUNT - 1]; // 2 of 3 are cbv
	UINT rootParameterCount = 0;

	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::OBJECT].ShaderRegister = UNIFORM_SLOT::OBJECT;
	rootParameters[UNIFORM_SLOT::OBJECT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::OBJECT].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::OBJECT]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::OBJECT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::CAMERA].ShaderRegister = UNIFORM_SLOT::CAMERA;
	rootParameters[UNIFORM_SLOT::CAMERA].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::CAMERA].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::CAMERA]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::CAMERA].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::FRAME].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::FRAME].ShaderRegister = UNIFORM_SLOT::FRAME;
	rootParameters[UNIFORM_SLOT::FRAME].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::FRAME].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::FRAME]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::FRAME].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	rootCBVDescriptors[UNIFORM_SLOT::SCENE].RegisterSpace = 0;
	rootCBVDescriptors[UNIFORM_SLOT::SCENE].ShaderRegister = UNIFORM_SLOT::SCENE;
	rootParameters[UNIFORM_SLOT::SCENE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[UNIFORM_SLOT::SCENE].Descriptor = rootCBVDescriptors[UNIFORM_SLOT::SCENE]; // this is the root descriptor for this root parameter
	rootParameters[UNIFORM_SLOT::SCENE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	rootParameterCount++;

	if (descriptorNum > 0)
	{
		// fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
		// buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
		rootParameters[UNIFORM_SLOT::TABLE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		rootParameters[UNIFORM_SLOT::TABLE].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		rootParameters[UNIFORM_SLOT::TABLE].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
		rootParameterCount++;
	}

	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC samplerWrap = {};
	samplerWrap.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //D3D12_FILTER_MIN_MAG_MIP_POINT;// 
	samplerWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerWrap.MipLODBias = 0;
	samplerWrap.MaxAnisotropy = 0;
	samplerWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerWrap.MinLOD = 0.0f;
	samplerWrap.MaxLOD = D3D12_FLOAT32_MAX;
	samplerWrap.ShaderRegister = 0;
	samplerWrap.RegisterSpace = 0;
	samplerWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC samplers[] = { samplerWrap };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(rootParameterCount, // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		_countof(samplers), // we have one static sampler
		samplers, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature));
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

ID3D12PipelineState* Renderer::GetFluidPSO(int frame, int index)
{
	return fluidPSO[frame][index];
}

ID3D12PipelineState** Renderer::GetFluidPsoPtr(int frame, int index)
{
	return &fluidPSO[frame][index];
}

ID3D12RootSignature* Renderer::GetFluidRootSignature(int frame, int index)
{
	return fluidRootSignature[frame][index];
}

ID3D12RootSignature** Renderer::GetFluidRootSignaturePtr(int frame, int index)
{
	return &fluidRootSignature[frame][index];
}

ID3D12DescriptorHeap* Renderer::GetFluidDescriptorHeap(int frame, int index)
{
	return  fluidDescriptorHeap[frame][index];
}

ID3D12DescriptorHeap** Renderer::GetFluidDescriptorHeapPtr(int frame, int index)
{
	return  &fluidDescriptorHeap[frame][index];
}

ID3D12DescriptorHeap* Renderer::GetFluidRtvDescriptorHeap(int frame, int index)
{
	return  fluidRtvDescriptorHeap[frame][index];
}

ID3D12DescriptorHeap** Renderer::GetFluidRtvDescriptorHeapPtr(int frame, int index)
{
	return  &fluidRtvDescriptorHeap[frame][index];
}

ID3D12DescriptorHeap* Renderer::GetFluidDsvDescriptorHeap(int frame, int index)
{
	return  fluidDsvDescriptorHeap[frame][index];
}

ID3D12DescriptorHeap** Renderer::GetFluidDsvDescriptorHeapPtr(int frame, int index)
{
	return  &fluidDsvDescriptorHeap[frame][index];
}
