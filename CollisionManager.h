#pragma once

#include "Singleton.h"
#include "Defines.h"

class CollisionComponent;

struct HitResult
{
	bool isColliding = false;

	Vector collisionNormal; // 충돌한 면의 법선 벡터 
	Vector collisionPoint;  // 충돌이 발생한 지점
	float penetrationDepth;
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
	void ProcessPredictiveCollisions();
	void CorrectPositionBeforeCollision(CollisionComponent* movingComp, CollisionComponent* staticComp, HitResult& predictedHit, Vector& velocity);

	HitResult CheckAABBCollision(CollisionComponent* comp1, CollisionComponent* comp2);
	HitResult CheckPredictiveCollision(CollisionComponent* movingComp, CollisionComponent* staticComp, Vector predictedPos);

	Vector GetComponentVelocity(CollisionComponent* comp);

	vector<CollisionComponent*> DetectEnemiesInRange(Vector playerPos,float radius,CollisionComponent* playerComp = nullptr);

private:
	bool IsComponentInCircle(CollisionComponent* comp, Vector centerPos, float radiusSquared);

private:
	bool bIgnore[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
	bool bBlock[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
	bool bOverlap[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };

	vector<CollisionComponent*> collisionComponents;
	unordered_map<pair<CollisionComponent*, CollisionComponent*>, bool, PairHash> collisionPairs;
	unordered_map<pair<CollisionComponent*, CollisionComponent*>, HitResult, PairHash> lastCollisionInfo;
};