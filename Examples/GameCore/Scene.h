#pragma once
#include "D3D12Core/EnginePch.h"
namespace dengine {
class Scene
{
public:
	shared_ptr<class Camera> GetMainCamera();

	void Render();

	void ClearRTV();


private:
	void PushLightData();

private:
	//vector<shared_ptr<GameObject>>		_gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;
};


}
