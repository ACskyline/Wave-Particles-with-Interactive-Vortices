#include "Mesh.h"

float Mesh::RayPlaneIntersection(XMFLOAT3 ori, XMFLOAT3 dir, XMFLOAT3 nor, XMFLOAT3 p)
{
	float t = -1;
	XMVECTOR oriV = XMLoadFloat3(&ori);
	XMVECTOR dirV = XMLoadFloat3(&dir);
	XMVECTOR norV = XMLoadFloat3(&nor);
	XMVECTOR pV = XMLoadFloat3(&p);
	XMStoreFloat(&t, XMVector3Dot(dirV, norV));
	if (t == 0) return -1;
	XMStoreFloat(&t, XMVector3Dot(pV - oriV, norV) / XMVector3Dot(dirV, norV));
	return t;
}

Mesh::Mesh(const MeshType& _type, 
	const XMFLOAT3& _position,
	const XMFLOAT3& _rotation,
	const XMFLOAT3& _scale) :
	type(_type),
	position(_position),
	scale(_scale),
	rotation(_rotation)
{
	if (type == MeshType::Plane)
	{
		InitPlane();
	}
	else if (type == MeshType::Cube)
	{
		InitCube();
	}
	else if (type == MeshType::FullScreenQuad)
	{
		InitFullScreenQuad();
	}
}

Mesh::Mesh(const MeshType& _type,
	int waveParticleCountOrSegment,
	const XMFLOAT3& _position,
	const XMFLOAT3& _rotation,
	const XMFLOAT3& _scale) :
	type(_type),
	position(_position),
	scale(_scale),
	rotation(_rotation)
{
	if (type == MeshType::WaveParticle)
	{
		InitWaveParticles(waveParticleCountOrSegment);
	}
	else if (type == MeshType::Circle)
	{
		InitCircle(waveParticleCountOrSegment);
	}
}

Mesh::Mesh(const MeshType& _type,
	int cellCountX,
	int cellCountZ,
	const XMFLOAT3& _position,
	const XMFLOAT3& _rotation,
	const XMFLOAT3& _scale) :
	type(_type),
	position(_position),
	scale(_scale),
	rotation(_rotation)
{
	if (type == MeshType::TileableSurface)
	{
		InitWaterSurface(cellCountX, cellCountZ);
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
		XMMatrixRotationX(XMConvertToRadians(rotation.x)) *
		XMMatrixRotationY(XMConvertToRadians(rotation.y)) *
		XMMatrixRotationZ(XMConvertToRadians(rotation.z)) *
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
	ID3D12Fence* fence;
	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue; // this value is incremented each frame. each fence will have its own value

	// -- Create fence related resources -- //
	fenceValue = 0; // set the initial fence value to 0
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
	{
		return false;
	}
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	// -- Create a direct command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&immediateCopyCommandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}
	immediateCopyCommandQueue->SetName(L"mesh immediate copy vertex");

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

	// -- Use fence to wait until finish -- //
	fenceValue++;
	hr = immediateCopyCommandQueue->Signal(fence, fenceValue);
	if (FAILED(hr))
	{
		return false;
	}

	if (fence->GetCompletedValue() < fenceValue)
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(hr))
		{
			return false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// -- Release -- //
	SAFE_RELEASE(fence);
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
	ID3D12Fence* fence;
	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue; // this value is incremented each frame. each fence will have its own value

	// -- Create fence related resources -- //
	fenceValue = 0; // set the initial fence value to 0
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
	{
		return false;
	}
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	// -- Create a direct command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&immediateCopyCommandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}
	immediateCopyCommandQueue->SetName(L"mesh immediate copy index");
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
	
	// -- Use fence to wait until finish -- //
	fenceValue++;
	hr = immediateCopyCommandQueue->Signal(fence, fenceValue);
	if (FAILED(hr))
	{
		return false;
	}

	if (fence->GetCompletedValue() < fenceValue)
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(hr))
		{
			return false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// -- Release -- //
	SAFE_RELEASE(fence);
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


D3D_PRIMITIVE_TOPOLOGY Mesh::GetPrimitiveType()
{
	return primitiveType;
}

void Mesh::SetPosition(const XMFLOAT3& _position)
{
	position = _position;
}

void Mesh::SetRotation(const XMFLOAT3& _rotation)
{
	rotation = _rotation;
}

XMFLOAT3 Mesh::GetPosition()
{
	return position;
}

XMFLOAT3 Mesh::GetRotation()
{
	return rotation;
}

int Mesh::GetIndexCount()
{
	return iList.size();
}

void Mesh::InitCube()
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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

void Mesh::InitPlane()
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vList.resize(4);

	// front face
	vList[0] = { -0.5f,  0.f, -0.5f, 0.0f, 1.0f };
	vList[1] = { -0.5f, 0.f, 0.5f, 0.0f, 0.0f };
	vList[2] = { 0.5f, 0.f, 0.5f, 1.0f, 0.0f };
	vList[3] = { 0.5f,  0.f, -0.5f, 1.0f, 1.0f };

	iList.resize(6);

	// front face 
	// first triangle
	iList[0] = 0;
	iList[1] = 1;
	iList[2] = 2;
	// second triangle
	iList[3] = 0;
	iList[4] = 2;
	iList[5] = 3;
}

void Mesh::InitWaveParticles(int waveParticleCount)
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	vList.resize(waveParticleCount);
	iList.resize(waveParticleCount);

	for (int i = 0; i < waveParticleCount; i++)
	{
		int index = i;
		XMFLOAT2 position = { rand() / (float)RAND_MAX * 2.f - 1.f, rand() / (float)RAND_MAX * 2.f - 1.f };
		XMFLOAT2 direction = { rand() / (float)RAND_MAX * 2.f - 1.f, rand() / (float)RAND_MAX * 2.f - 1.f };
		XMStoreFloat2(&direction, XMVector2Normalize(XMLoadFloat2(&direction)));
		float height = rand() / (float)RAND_MAX * 0.1 + 0.2;
		float radius = rand() / (float)RAND_MAX * 0.05 + 0.1;
		float beta = rand() / (float)RAND_MAX;
		float speed = rand() / (float)RAND_MAX;
		vList[index] = { position.x, position.y, height, direction.x, direction.y, radius, beta, speed };
		iList[index] = index;
	}
}

void Mesh::InitWaterSurface(int cellCountX, int cellCountZ)
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	int cellCount = cellCountX * cellCountZ;
	int vertexCount = (cellCountX + 1) * (cellCountZ + 1);
	vList.resize(vertexCount);
	iList.resize(cellCount * 4);

	for (int i = 0; i <= cellCountX; i++)
	{
		for (int j = 0; j <= cellCountZ; j++)
		{
			int vIndex = i * (cellCountZ + 1) + j;
			// make the border wider so that it can sample outside of [0, 1] and return border color which is set to transparent black
			float ui = i;
			float vj = j;
			if (i == 0) ui-= EPSILON;
			else if (i == cellCountX) ui+= EPSILON;
			if (j == 0) vj-= EPSILON;
			else if (j == cellCountZ) vj+= EPSILON;
			vList[vIndex] = { i / (float)cellCountX, 0, j / (float)cellCountZ, ui / (float)cellCountX, vj / (float)cellCountZ };
		}
	}

	for (int i = 0; i < cellCountX; i++)
	{
		for (int j = 0; j < cellCountZ; j++)
		{
			int cIndex = i * cellCountZ + j;
			iList[cIndex * 4 + 0] = i * (cellCountZ + 1) + j;
			iList[cIndex * 4 + 1] = i * (cellCountZ + 1) + j + 1;
			iList[cIndex * 4 + 2] = (i + 1) * (cellCountZ + 1) + j + 1;
			iList[cIndex * 4 + 3] = (i + 1) * (cellCountZ + 1) + j;
		}
	}
}

void Mesh::InitFullScreenQuad()
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vList.resize(4);

	// front face
	vList[0] = { -1.f,  -1.f, 0.5f, 0.0f, 1.0f };
	vList[1] = { -1.f, 1.f, 0.5f, 0.0f, 0.0f };
	vList[2] = { 1.f, 1.f, 0.5f, 1.0f, 0.0f };
	vList[3] = { 1.f,  -1.f, 0.5f, 1.0f, 1.0f };

	iList.resize(6);

	// front face 
	// first triangle
	iList[0] = 0;
	iList[1] = 1;
	iList[2] = 2;
	// second triangle
	iList[3] = 0;
	iList[4] = 2;
	iList[5] = 3;
}

void Mesh::InitCircle(int segment)
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float angle = 360.0 / segment;

	vList.resize(3 * segment);

	for (int i = 0; i < segment; i++)
	{
		float sini = 0, cosi = 0, sinj = 0, cosj = 0;
		XMScalarSinCos(&sini, &cosi, XMConvertToRadians(angle * i));
		XMScalarSinCos(&sinj, &cosj, XMConvertToRadians(angle * (i + 1)));
		vList[i * 3 + 0] = { 0.f, 0.5f, 0.f, 0.5f, 0.5f };
		vList[i * 3 + 1] = { cosj, sinj, 0.5f, cosj * 0.5f + 0.5f, sinj * 0.5f + 0.5f };
		vList[i * 3 + 2] = { cosi, sini, 0.5f, cosi * 0.5f + 0.5f, sini * 0.5f + 0.5f };
	}

	iList.resize(3 * segment);

	for (int i = 0; i < segment; i++)
	{
		iList[i * 3 + 0] = i * 3 + 0;
		iList[i * 3 + 1] = i * 3 + 1;
		iList[i * 3 + 2] = i * 3 + 2;
	}
}

void Mesh::PrintWaveParticle()
{
	for (int i = 0; i < vList.size(); i++)
	{
		printf("%d: %f\n", i, vList[i].pos.z);
	}
}
