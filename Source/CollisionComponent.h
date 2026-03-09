#pragma once

#include "Component.h"
#include "CollisionManager.h"

class CollisionComponent : public Component
{
	using Super = Component;
	using FOnComponentBeginOverlap = function<void(CollisionComponent*, HitResult)>;
	using FOnComponentEndOverlap = function<void(CollisionComponent*, HitResult)>;

public:
	void Init(Actor* _owner) override;
	void Update(float deltaTime) override;
	void Render(HDC hdc) override;
	void Destroy() override;

	void SetCollisionSize(int _offsetX, int _offsetY);
	void SetPosition(Vector newPosition) { position = newPosition; }

	void SetPendginKill(bool state) { bPendingKill = state; }
	bool IsPendingKill() { return bPendingKill; }

	void SetCollisionChannel(ECollisionChannel channel) { collisionChannel = channel; }
	ECollisionChannel GetCollisionChannel() { return collisionChannel; }

	RECT GetBoundingBox() const;

public:
	FOnComponentBeginOverlap OnComponentBeginOverlap;
	FOnComponentEndOverlap OnComponentEndOverlap;

private:
	Vector position;
	int offsetX;
	int offsetY;
	int width;  
	int height;  

	bool bPendingKill = false;

	ECollisionChannel collisionChannel;
};

