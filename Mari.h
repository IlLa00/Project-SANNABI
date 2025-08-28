#pragma once

#include "Actor.h"

class SpriteRenderComponent;
class CollisionComponent;
class TextureResource;

class Mari : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void OnBeginOverlap(CollisionComponent* other, HitResult info);

private:
	SpriteRenderComponent* bodyComponent = nullptr;
	CollisionComponent* collisionComponent = nullptr;
	TextureResource* endTextrue = nullptr;

	float endGameTimer = 0.f;
	bool bEndGame = false;
	bool bShowGameOverScreen = false;
};

