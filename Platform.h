#pragma once

#include "Actor.h"
#include "CollisionManager.h"

class CollisionComponent;
class TextureResource;
class GrapplingHookProjectile;

class Platform : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;

	void SetLimitPoint(float limitMinY, float limitMaxY);

	void OnBeginOverlap(CollisionComponent* other, HitResult info);

private:
	CollisionComponent* collisionComponent = nullptr;
	TextureResource* texture = nullptr;
	GrapplingHookProjectile* projectile = nullptr;

	float fallingSpeed = 100.f;
	bool bFalling = false;

	float _limitMinY;
	float _limitMaxY;
};

