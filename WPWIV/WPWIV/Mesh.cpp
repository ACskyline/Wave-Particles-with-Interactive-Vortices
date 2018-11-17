#include "Mesh.h"

Mesh::Mesh(MeshType _type, 
	const XMFLOAT3& _position,
	const XMFLOAT3& _scale,
	const XMFLOAT3& _rotation) :
	type(_type),
	position(_position),
	scale(_scale),
	rotation(_rotation)
{
	if (type == MeshType::Plane)
	{

	}
	else if (type == MeshType::Cube)
	{
		InitCube();
	}
}

Mesh::~Mesh()
{
	ReleaseBuffers();
}

void Mesh::ResetMesh(MeshType _type,
	const XMFLOAT3& _position,
	const XMFLOAT3& _scale,
	const XMFLOAT3& _rotation)
{
	type = _type;
	position = _position;
	scale = _scale;
	rotation = _rotation;
	if (type == MeshType::Plane)
	{

	}
	else if (type == MeshType::Cube)
	{
		InitCube();
	}
}

bool Mesh::InitMesh(ID3D12Device* device)
{
	UpdateUniform();
	if(!CreateUniformBuffer(device))
		return false;
	UpdateUniformBuffer();
	if (!CreateVertexBuffer(device))
		return false;
	if (!CreateIndexBuffer(device))
		return false;
	if (!UpdateVertexBuffer(device))
		return false;
	if (!UpdateIndexBuffer(device))
		return false;
	return true;
}

void Mesh::UpdateUniform()
{
	XMStoreFloat4x4(&uniform.model,
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationX(rotation.x) *
		XMMatrixRotationY(rotation.y) *
		XMMatrixRotationZ(rotation.z) *
		XMMatrixTranslation(position.x, position.y, position.z));

	XMStoreFloat4x4(&uniform.modelInv, XMMatrixInverse(nullptr, XMLoadFloat4x4(&uniform.model)));
}

bool Mesh::CreateUniformBuffer(ID3D12Device* device)
{
	HRESULT hr;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ObjectUniform)), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
		nullptr, // we do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&gpuUniformBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	gpuUniformBuffer->SetName(L"object uniform buffer");

	CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

	// map the resource heap to get a gpu virtual address to the beginning of the heap
	hr = gpuUniformBuffer->Map(0, &readRange, &cpuUniformBufferAddress);

	return true;
}

void Mesh::UpdateUniformBuffer()
{
	memcpy(cpuUniformBufferAddress, &uniform, sizeof(uniform));
}

bool Mesh::CreateVertexBuffer(ID3D12Device* device)
{
	HRESULT hr;

	vertexBufferSizeInBytes = sizeof(Vertex) * vList.size();

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&gpuVertexBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	gpuVertexBuffer->SetName(L"mesh vertex buffer");

	vertexBufferView.BufferLocation = gpuVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSizeInBytes;

	return true;
}

bool Mesh::CreateIndexBuffer(ID3D12Device* device)
{
	HRESULT hr;

	indexBufferSizeInBytes = sizeof(uint32_t) * iList.size();

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&gpuIndexBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	gpuIndexBuffer->SetName(L"mesh index buffer");

	indexBufferView.BufferLocation = gpuIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer
	indexBufferView.SizeInBytes = indexBufferSizeInBytes;

	return true;
}

bool Mesh::UpdateVertexBuffer(ID3D12Device* device)
{
	HRESULT hr;

	ID3D12CommandQueue* immediateCopyCommandQueue;
	ID3D12CommandAllocator* immediateCopyCommandAllocator;
	ID3D12GraphicsCommandList* immediateCopyCommandList;
	ID3D12Resource* immediateCopyBuffer;

	// -- Create a direct command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&immediateCopyCommandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a command allocator -- //
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&immediateCopyCommandAllocator));
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a command list -- //
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, immediateCopyCommandAllocator, NULL, IID_PPV_ARGS(&immediateCopyCommandList));
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create an upload buffer -- //
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&immediateCopyBuffer));
	if (FAILED(hr))
	{
		return false;
	}
	immediateCopyBuffer->SetName(L"vertex upload buffer");

	// store vertex data in upload buffer
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = static_cast<void*>(vList.data()); // pointer to our vertex array
	vertexData.RowPitch = vertexBufferSizeInBytes; // size of all our triangle vertex data
	vertexData.SlicePitch = vertexBufferSizeInBytes; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(immediateCopyCommandList, gpuVertexBuffer, immediateCopyBuffer, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	immediateCopyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	
	immediateCopyCommandList->Close();
	ID3D12CommandList* commandLists[] = { immediateCopyCommandList };
	immediateCopyCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	SAFE_RELEASE(immediateCopyCommandQueue);
	SAFE_RELEASE(immediateCopyCommandAllocator);
	SAFE_RELEASE(immediateCopyCommandList);
	SAFE_RELEASE(immediateCopyBuffer);

	return true;
}

bool Mesh::UpdateIndexBuffer(ID3D12Device* device)
{
	HRESULT hr;

	ID3D12CommandQueue* immediateCopyCommandQueue;
	ID3D12CommandAllocator* immediateCopyCommandAllocator;
	ID3D12GraphicsCommandList* immediateCopyCommandList;
	ID3D12Resource* immediateCopyBuffer;

	// -- Create a direct command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&immediateCopyCommandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a command allocator -- //
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&immediateCopyCommandAllocator));
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a command list -- //
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, immediateCopyCommandAllocator, NULL, IID_PPV_ARGS(&immediateCopyCommandList));
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create an upload buffer -- //
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&immediateCopyBuffer));
	if (FAILED(hr))
	{
		return false;
	}
	immediateCopyBuffer->SetName(L"index upload buffer");

	// store vertex data in upload buffer
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = static_cast<void*>(iList.data()); // pointer to our vertex array
	indexData.RowPitch = indexBufferSizeInBytes; // size of all our triangle vertex data
	indexData.SlicePitch = indexBufferSizeInBytes; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(immediateCopyCommandList, gpuIndexBuffer, immediateCopyBuffer, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	immediateCopyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	immediateCopyCommandList->Close();
	ID3D12CommandList* commandLists[] = { immediateCopyCommandList };
	immediateCopyCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	SAFE_RELEASE(immediateCopyCommandQueue);
	SAFE_RELEASE(immediateCopyCommandAllocator);
	SAFE_RELEASE(immediateCopyCommandList);
	SAFE_RELEASE(immediateCopyBuffer);

	return true;
}

void Mesh::ReleaseBuffers()
{
	SAFE_RELEASE(gpuVertexBuffer);
	SAFE_RELEASE(gpuIndexBuffer);
	SAFE_RELEASE(gpuUniformBuffer);
}

D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexBufferView()
{
	return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView()
{
	return indexBufferView;
}

D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetUniformBufferGpuAddress()
{
	return gpuUniformBuffer->GetGPUVirtualAddress();
}

void Mesh::SetPosition(const XMFLOAT3& _position)
{
	position = _position;
}

XMFLOAT3 Mesh::GetPosition()
{
	return position;
}

void Mesh::InitCube()
{
	vList.resize(24);

	// front face
	vList[0] = { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f };
	vList[1] = { 0.5f, -0.5f, -0.5f, 1.0f, 1.0f };
	vList[2] = { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f };
	vList[3] = { 0.5f,  0.5f, -0.5f, 1.0f, 0.0f };

	// right side face
	vList[4] = { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f };
	vList[5] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f };
	vList[6] = { 0.5f, -0.5f,  0.5f, 1.0f, 1.0f };
	vList[7] = { 0.5f,  0.5f, -0.5f, 0.0f, 0.0f };

	// left side face
	vList[8] = { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f };
	vList[9] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f };
	vList[10] = { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f };
	vList[11] = { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f };

	// back face
	vList[12] = { 0.5f,  0.5f,  0.5f, 0.0f, 0.0f };
	vList[13] = { -0.5f, -0.5f,  0.5f, 1.0f, 1.0f };
	vList[14] = { 0.5f, -0.5f,  0.5f, 0.0f, 1.0f };
	vList[15] = { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f };

	// top face
	vList[16] = { -0.5f,  0.5f, -0.5f, 0.0f, 1.0f };
	vList[17] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f };
	vList[18] = { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f };
	vList[19] = { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f };

	// bottom face
	vList[20] = { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f };
	vList[21] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f };
	vList[22] = { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f };
	vList[23] = { -0.5f, -0.5f,  0.5f, 1.0f, 0.0f };

	iList.resize(36);

	// front face 
	// first triangle
	iList[0] = 0;
	iList[1] = 1;
	iList[2] = 2;
	// second triangle
	iList[3] = 0;
	iList[4] = 3;
	iList[5] = 1;

	// left face 
	// first triangle
	iList[6] = 4;
	iList[7] = 5;
	iList[8] = 6;
	// second triangle
	iList[9] = 4;
	iList[10] = 7;
	iList[11] = 5;

	// right face 
	// first triangle
	iList[12] = 8;
	iList[13] = 9;
	iList[14] = 10;
	// second triangle
	iList[15] = 8;
	iList[16] = 11;
	iList[17] = 9;

	// back face 
	// first triangle
	iList[18] = 12;
	iList[19] = 13;
	iList[20] = 14;
	// second triangle
	iList[21] = 12;
	iList[22] = 15;
	iList[23] = 13;

	// top face 
	// first triangle
	iList[24] = 16;
	iList[25] = 17;
	iList[26] = 18;
	// second triangle
	iList[27] = 16;
	iList[28] = 19;
	iList[29] = 17;

	// bottom face
	// first triangle
	iList[30] = 20;
	iList[31] = 21;
	iList[32] = 22;
	// second triangle
	iList[33] = 20;
	iList[34] = 23;
	iList[35] = 21;
}