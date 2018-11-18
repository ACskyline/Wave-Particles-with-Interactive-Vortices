#pragma once

#include "Camera.h"
#include "Mesh.h"

class Scene
{
public:
	Scene();
	~Scene();

	Camera* pCamera;
	vector<Mesh*> pMeshVec;
};

