#pragma once

#include "Singleton.h"
#include "Defines.h"

class CollisionComponent;

struct HitResult
{
	bool isColliding = false;

	Vector collisionNormal; // 충돌한 면의 법선 벡터 
	Vector collisionPoint;  // 충돌이 발생한 지점
};

class CollisionManager : public Singleton<CollisionManager>
{
	friend Singleton<CollisionManager>;

private:
	CollisionManager() {}

public:
	void Init(HWND hwnd);
	void Update();
	void Destroy();

	void RemovePendingKillComponents();

	void RegisterCollisionComponent(CollisionComponent* comp);
	void UnregisterCollisionComponent(CollisionComponent* comp);

	HitResult CheckAABBCollision(CollisionComponent* comp1, CollisionComponent* comp2);

private:
	bool bIgnore[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
	bool bBlock[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
	bool bOverlap[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };

	vector<CollisionComponent*> collisionComponents;
	unordered_map<pair<CollisionComponent*, CollisionComponent*>, bool, PairHash> collisionPairs;
	unordered_map<pair<CollisionComponent*, CollisionComponent*>, HitResult, PairHash> lastCollisionInfo;
};