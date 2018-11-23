#pragma once

#include "Frame.h"
#include "Shader.h"

class Scene
{
public:
	Scene();
	~Scene();
	void AddFrame(Frame* pFrame);
	void AddCamera(Camera* pCamera);
	void AddMesh(Mesh* pMesh);
	void AddTexture(Texture* pTexture);
	void AddShader(Shader* pShader);
	void AddRenderTexture(RenderTexture* pRenderTexture);

	bool LoadScene();
	bool InitScene(ID3D12Device* device);

private:
	vector<Frame*> pFrameVec;
	vector<Camera*> pCameraVec;
	vector<Mesh*> pMeshVec;
	vector<Texture*> pTextureVec;
	vector<Shader*> pShaderVec;
	vector<RenderTexture*> pRenderTextureVec;
};
