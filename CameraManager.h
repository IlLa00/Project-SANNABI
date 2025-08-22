#pragma once

#include "Singleton.h"

class CameraManager : public Singleton<CameraManager>
{
	friend Singleton<CameraManager>;

private:
	CameraManager() {}

public:
	void Update(Vector playerPos, Vector mapSize, float deltaTime);
	void UpdateCameraShake(float deltaTime);
	void StartCameraShake(float duration, float intensity);
	void StopCameraShake();

	Vector ConvertScreenPos(Vector worldPos);
	Vector ConvertWorldPos(Vector screenPos);

	void SetCameraPos(Vector pos) { cameraPos = pos; }
	Vector GetCameraPos() { return cameraPos; }

private:
	Vector cameraPos = Vector(0, 0);

	bool isShaking = false;
	float shakeTimer = 0.0f;
	float shakeDuration = 0.0f;
	float shakeIntensity = 0.0f;
	Vector shakeOffset = { 0, 0 };
};


