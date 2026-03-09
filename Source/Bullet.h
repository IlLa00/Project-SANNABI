#pragma once

#include "Actor.h"

class CollisionComponent;
class TextureResource;

class Bullet : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void Activate(Vector position, Vector direction, float speed);
	void Deactivate();

	bool IsActive() const { return bActive; }

private:
	CollisionComponent* collisionComponent = nullptr;
	TextureResource* texture = nullptr;

	Vector direction;
	float speed;
	float rotationAngle;
	bool bActive = false;
	bool bFlying = false;
};

