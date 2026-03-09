#include "pch.h"
#include "CameraManager.h"

void CameraManager::Update(Vector playerPos, Vector mapSize, float deltaTime)
{
	cameraPos = playerPos;

	float halfSizeX = GWinSizeX / 2;
	float halfSizeY = GWinSizeY / 2;

	cameraPos.x = clamp(cameraPos.x, halfSizeX, mapSize.x - halfSizeX);
	cameraPos.y = clamp(cameraPos.y, halfSizeY, mapSize.y - halfSizeY);

	UpdateCameraShake(deltaTime);
}

void CameraManager::UpdateCameraShake(float deltaTime)
{
	if (!isShaking) return;

	shakeTimer -= deltaTime;

	if (shakeTimer <= 0)
	{
		isShaking = false;
		shakeOffset = { 0, 0 };
	}
	else
	{
		float normalizedTime = shakeTimer / shakeDuration;
		float currentIntensity = shakeIntensity * normalizedTime; // 시간에 따라 감소

		float randomX = ((rand() % 1000) / 1000.0f * 2.0f - 1.0f) * currentIntensity; // -intensity ~ +intensity
		float randomY = ((rand() % 1000) / 1000.0f * 2.0f - 1.0f) * currentIntensity;

		shakeOffset.x = randomX;
		shakeOffset.y = randomY;

		cameraPos += shakeOffset;
	}
}

void CameraManager::StartCameraShake(float duration, float intensity)
{
	isShaking = true;
	shakeTimer = duration;
	shakeDuration = duration;
	shakeIntensity = intensity;
	shakeOffset = { 0, 0 };
}

void CameraManager::StopCameraShake()
{
	isShaking = false;
	shakeTimer = 0.0f;
	shakeOffset = { 0, 0 };
}

Vector CameraManager::ConvertScreenPos(Vector worldPos)
{
	Vector convertPos;

	convertPos.x = worldPos.x - (cameraPos.x - (GWinSizeX / 2));
	convertPos.y = worldPos.y - (cameraPos.y - (GWinSizeY / 2));

	return convertPos;
}

Vector CameraManager::ConvertWorldPos(Vector screenPos)
{
	Vector convertPos;

	convertPos.x = screenPos.x + (cameraPos.x - (GWinSizeX / 2));
	convertPos.y = screenPos.y + (cameraPos.y - (GWinSizeY / 2));

	return convertPos;
}
