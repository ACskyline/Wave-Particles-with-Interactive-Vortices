#pragma once

#include "GlobalInclude.h"

class Mesh
{
public:
	enum MeshType { Plane, Cube, WaveParticle, WaterSurface, FullScreenQuad };

	struct ObjectUniform
	{
		XMFLOAT4X4 model;
		XMFLOAT4X4 modelInv;
	};

	Mesh(const MeshType& _type,
		const XMFLOAT3& _position,
		const XMFLOAT3& _rotation,
		const XMFLOAT3& _scale);
	Mesh(const MeshType& _type,
		int waveParticleCount,
		const XMFLOAT3& _position,
		const XMFLOAT3& _rotation,
		const XMFLOAT3& _scale);
	Mesh(const MeshType& _type,
		int cellCountX,
		int cellCountZ,
		const XMFLOAT3& _position,
		const XMFLOAT3& _rotation,
		const XMFLOAT3& _scale);
	~Mesh();

	void ResetMesh(MeshType _type,
		const XMFLOAT3& _position,
		const XMFLOAT3& _scale,
		const XMFLOAT3& _rotation);

	bool InitMesh(ID3D12Device* device);

	void UpdateUniform();
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();

	bool CreateVertexBuffer(ID3D12Device* device);
	bool CreateIndexBuffer(ID3D12Device* device);
	bool UpdateVertexBuffer(ID3D12Device* device);
	bool UpdateIndexBuffer(ID3D12Device* device);

	void ReleaseBuffers();

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();
	D3D_PRIMITIVE_TOPOLOGY GetPrimitiveType();

	void SetPosition(const XMFLOAT3& _position);
	void SetRotation(const XMFLOAT3& _rotation);
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();
	int GetIndexCount();

private:
	MeshType type;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 rotation;

	vector<Vertex> vList;
	vector<uint32_t> iList;

	ObjectUniform uniform;
	ID3D12Resource* gpuVertexBuffer;
	ID3D12Resource* gpuIndexBuffer;
	ID3D12Resource* gpuUniformBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	void* cpuUniformBufferAddress;
	D3D_PRIMITIVE_TOPOLOGY primitiveType;

	UINT vertexBufferSizeInBytes;
	UINT indexBufferSizeInBytes;

	void InitCube();
	void InitPlane();
	void InitWaveParticles(int waveParticleCount);
	void InitWaterSurface(int cellCountX, int cellCountZ);
	void InitFullScreenQuad();
};

