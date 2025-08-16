#pragma once

#include "Actor.h"

class CollisionComponent;

class Bullet : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;

	void Activate(Vector position, Vector direction, float speed);
	void Deactivate();

	bool IsActive() const { return bActive; }

private:
	CollisionComponent* collsionComponent = nullptr;

	Vector direction;
	float speed;
	bool bActive = false;
	bool bFlying = false;
};

