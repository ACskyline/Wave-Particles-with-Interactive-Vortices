#pragma once

#include "GlobalInclude.h"

class Camera
{
public:
	struct CameraUniform
	{
		XMFLOAT4X4 viewProj;
		XMFLOAT4X4 viewProjInv;
		float vx;
		float vy;
		float vz;
	};

	Camera(const XMFLOAT3 &_position,
		const XMFLOAT3 &_target, 
		const XMFLOAT3 &_up,
		float _width,
		float _height,
		float _fov,
		float _nearClipPlane,
		float _farClipPlane);
	virtual ~Camera();

	void ResetCamera(const XMFLOAT3 &_position,
		const XMFLOAT3 &_target,
		const XMFLOAT3 &_up,
		float _width,
		float _height,
		float _fov,
		float _nearClipPlane,
		float _farClipPlane);

	bool InitCamera(ID3D12Device* device);

	void UpdateViewport();
	void UpdateScissorRect();
	void UpdateUniform();
	bool CreateUniformBuffer(ID3D12Device* device);
	void UpdateUniformBuffer();
	void ReleaseBuffer();
	XMFLOAT3 GetPosition();

	D3D12_GPU_VIRTUAL_ADDRESS GetUniformBufferGpuAddress();
	D3D12_VIEWPORT GetViewport();
	D3D12_RECT GetScissorRect();

	XMFLOAT3 ScreenToWorld(XMFLOAT2 screenPos, bool nearClipPlane = false);

protected:
	float width;
	float height;
	float fov;
	float nearClipPlane;
	float farClipPlane;
	XMFLOAT3 position;
	XMFLOAT3 target;
	XMFLOAT3 up;

	CameraUniform uniform;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	ID3D12Resource* gpuUniformBuffer;
	ID3D12Resource* gpuVertexBuffer;
	ID3D12Resource* gpuIndexBuffer;
	void* cpuUniformBufferAddress;
};

class OrbitCamera :
	public Camera
{
public:
	OrbitCamera(float _distance,
		float _horizontalAngle,
		float _verticalAngle,
		const XMFLOAT3 &_target,
		const XMFLOAT3 &_up,
		float _width,
		float _height,
		float _fov,
		float _nearClipPlane,
		float _farClipPlane);
		
	virtual ~OrbitCamera();

	float GetDistance();
	float GetHorizontalAngle();
	float GetVerticalAngle();
	void SetDistance(float _distance);
	void SetHorizontalAngle(float _horizontalAngle);
	void SetVerticalAngle(float _verticalAngle);

private:
	float distance;
	float horizontalAngle;
	float verticalAngle;

	void UpdatePosition();
};

