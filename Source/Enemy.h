#pragma once

#include "Actor.h"
#include "CollisionManager.h"

class CollisionComponent;

class Enemy :public Actor
{
	using Super = Actor;

public:
	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC _hdcBack) override;
	virtual void Destroy() override;

	virtual void TakeDamage() override;

	virtual void OnCharacterBeginOverlap(CollisionComponent* other, HitResult info);
	virtual void OnCharacterEndOverlap(CollisionComponent* other, HitResult info);
	virtual void OnPerceiveCharacter(CollisionComponent* other, HitResult info);
	virtual void OffPerceiveCharacter(CollisionComponent* other, HitResult info);

	bool IsDetectTarget() { return bDetectTarget; }

protected:
	CollisionComponent* collisionComponent = nullptr;
	CollisionComponent* perceiveComponent = nullptr;
	Actor* target = nullptr;

	bool bDetectTarget = false;
	bool bDamaged = false;

	Vector dectionRange = Vector(500, 500);
};

