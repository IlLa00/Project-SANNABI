#pragma once

#include "Singleton.h"

class CameraManager : public Singleton<CameraManager>
{
	friend Singleton<CameraManager>;

private:
	CameraManager() {}

public:
	void Update(Vector playerPos, Vector mapSize);

	Vector ConvertScreenPos(Vector worldPos);
	Vector ConvertWorldPos(Vector screenPos);

	void SetCameraPos(Vector pos) { cameraPos = pos; }
	Vector GetCameraPos() { return cameraPos; }

private:
	Vector cameraPos = Vector(0, 0);
};


