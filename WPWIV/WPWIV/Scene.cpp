#include "Scene.h"



Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::AddFrame(Frame* pFrame)
{
	pFrameVec.push_back(pFrame);
}

void Scene::AddCamera(Camera* pCamera)
{
	pCameraVec.push_back(pCamera);
}

void Scene::AddMesh(Mesh* pMesh)
{
	pMeshVec.push_back(pMesh);
}

void Scene::AddTexture(Texture* pTexture)
{
	pTextureVec.push_back(pTexture);
}

void Scene::AddShader(Shader* pShader)
{
	pShaderVec.push_back(pShader);
}

void Scene::AddRenderTexture(RenderTexture* pRenderTexture)
{
	pRenderTextureVec.push_back(pRenderTexture);
}

bool Scene::LoadScene()
{
	// CPU side, load data from disk
	int shaderCount = pShaderVec.size();
	for (int i = 0; i < shaderCount; i++)
	{
		if (!pShaderVec[i]->CreateShader())
		{
			printf("CreateShader %d failed\n", i);
			return false;
		}
	}

	int textureCount = pTextureVec.size();
	for (int i = 0; i < textureCount; i++)
	{
		if (!pTextureVec[i]->LoadTextureBuffer())
		{
			printf("LoadTexture failed\n");
			return false;
		}
	}

	return true;
}

bool Scene::InitScene(ID3D12Device* device)
{
	int frameCount = pFrameVec.size();
	for (int i = 0; i < frameCount; i++)
	{
		if (!pFrameVec[i]->InitFrame(device))
		{
			printf("InitFrame failed\n");
			return false;
		}
	}

	int cameraCount = pCameraVec.size();
	for (int i = 0; i < cameraCount; i++)
	{
		if (!pCameraVec[i]->InitCamera(device))
		{
			printf("InitCamera failed\n");
			return false;
		}
	}

	int meshCount = pMeshVec.size();
	for (int i = 0; i < meshCount; i++)
	{
		if (!pMeshVec[i]->InitMesh(device))
		{
			printf("InitMesh failed\n");
			return false;
		}
	}

	int textureCount = pTextureVec.size();
	for (int i = 0; i < textureCount; i++)
	{
		if (!pTextureVec[i]->InitTexture(device))
		{
			printf("InitTexture failed\n");
			return false;
		}
	}

	int renderTextureCount = pRenderTextureVec.size();
	for (int i = 0; i < renderTextureCount; i++)
	{
		if (!pRenderTextureVec[i]->InitTexture(device))
		{
			printf("InitRenderTexture failed\n");
			return false;
		}
	}

	return true;
}
