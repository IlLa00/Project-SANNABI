#include "pch.h"
#include "CameraManager.h"

void CameraManager::Update(Vector playerPos, Vector mapSize)
{
	cameraPos = playerPos;

	float halfSizeX = GWinSizeX / 2;
	float halfSizeY = GWinSizeY / 2;

	cameraPos.x = clamp(cameraPos.x, halfSizeX, mapSize.x - halfSizeX);
	cameraPos.y = clamp(cameraPos.y, halfSizeY, mapSize.y - halfSizeY);
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
