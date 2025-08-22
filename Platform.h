#pragma once

#include "Actor.h"
#include "CollisionManager.h"

class CollisionComponent;
class TextureResource;

class Platform : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;

	void OnBeginOverlap(CollisionComponent* other, HitResult info);

private:
	CollisionComponent* collisionComponent = nullptr;
	TextureResource* texture = nullptr;

	float fallingSpeed = 100.f;
	bool bFalling = false;
};

