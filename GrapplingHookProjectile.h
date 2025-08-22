#pragma once

#include "Actor.h"

class CollisionComponent;
class TextureResource;

class GrapplingHookProjectile : public Actor
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

	bool IsFlying() const { return bFlying; }
	void SetFlying(bool newState) { bFlying = newState; }

private:
	CollisionComponent* collsionComponent = nullptr;
	TextureResource* texture = nullptr;

	Vector direction;
	float speed;
	bool bActive = false;
	bool bFlying = false;
};

