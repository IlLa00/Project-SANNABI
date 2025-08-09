#pragma once

#include "Actor.h"

class CollisionComponent;

class TestGround : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Render(HDC _hdcBack) override;

private:
	CollisionComponent* collisionComponent = nullptr;
};

