#include "pch.h"
#include "CollisionManager.h"
#include "Player.h"
#include "GrapplingHookProjectile.h"
#include "CollisionComponent.h"

void CollisionManager::Init(HWND hwnd)
{
	bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Character] = 1;
	bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::WorldStatic] = 1;
	bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::WorldDynamic] = 1;

	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::WorldStatic] = 1;
	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::WorldDynamic] = 1;
	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Projectile] = 1;
	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Character] = 1;
	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Perception] = 1;

	bBlock[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::Projectile] = 1;
	bBlock[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::Character] = 1;

	bBlock[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::Projectile] = 1;
	bBlock[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::Character] = 1;

	bBlock[(int)ECollisionChannel::Perception][(int)ECollisionChannel::Character] = 1;

	bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Projectile] = 1;

	bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldStatic] = 1;
	bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldDynamic] = 1;

	bIgnore[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::WorldDynamic] = 1;
	bIgnore[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::WorldStatic] = 1;


}

void CollisionManager::Update()
{
	for (int i = 0; i < collisionComponents.size(); i++)
	{
		if (!collisionComponents[i]->IsActive()) continue;

		for (int j = i+1; j < collisionComponents.size(); j++)
		{
			if (!collisionComponents[j]->IsActive()) continue;

			ECollisionChannel channel1 = collisionComponents[i]->GetCollisionChannel();
			ECollisionChannel channel2 = collisionComponents[j]->GetCollisionChannel();

			if (bIgnore[(int)channel1][(int)channel2] || !bBlock[(int)channel1][(int)channel2])
				continue;

			auto pair = make_pair(collisionComponents[i], collisionComponents[j]);
			HitResult collisionInfo = CheckAABBCollision(collisionComponents[i], collisionComponents[j]);
			bool wasColliding = collisionPairs[pair];

			if (collisionInfo.isColliding && !wasColliding) // 새로운 충돌 시작
			{
				lastCollisionInfo[pair] = collisionInfo;

				if (collisionComponents[i]->OnComponentBeginOverlap)
					collisionComponents[i]->OnComponentBeginOverlap(collisionComponents[j], collisionInfo);

				if (collisionComponents[j]->OnComponentBeginOverlap)
				{
					HitResult reversedInfo = collisionInfo;
					reversedInfo.collisionNormal.x = -collisionInfo.collisionNormal.x;
					reversedInfo.collisionNormal.y = -collisionInfo.collisionNormal.y;
					collisionComponents[j]->OnComponentBeginOverlap(collisionComponents[i], reversedInfo);
				}
			}
			else if (!collisionInfo.isColliding && wasColliding) // 충돌 종료
			{
				HitResult lastInfo = lastCollisionInfo[pair];

				if (collisionComponents[i]->OnComponentEndOverlap)
					collisionComponents[i]->OnComponentEndOverlap(collisionComponents[j], lastInfo);

				if (collisionComponents[j]->OnComponentEndOverlap)
				{
					HitResult reversedInfo = lastInfo;
					reversedInfo.collisionNormal.x = -lastInfo.collisionNormal.x;
					reversedInfo.collisionNormal.y = -lastInfo.collisionNormal.y;
					collisionComponents[j]->OnComponentEndOverlap(collisionComponents[i], reversedInfo);
				}

				lastCollisionInfo.erase(pair);
			}

			collisionPairs[pair] = collisionInfo.isColliding;
		}
	}
}

HitResult CollisionManager::CheckAABBCollision(CollisionComponent* comp1, CollisionComponent* comp2)
{
	HitResult info;

	if (!comp1 || !comp2) return info;

	RECT rect1 = comp1->GetBoundingBox();
	RECT rect2 = comp2->GetBoundingBox();

	if (rect1.right < rect2.left || rect1.left > rect2.right ||
		rect1.bottom < rect2.top || rect1.top > rect2.bottom)
		return info; 

	info.isColliding = true;

	float overlapX = min(rect1.right, rect2.right) - max(rect1.left, rect2.left);
	float overlapY = min(rect1.bottom, rect2.bottom) - max(rect1.top, rect2.top);

	// X축 겹침이 Y축 겹침보다 작다면, 수직 충돌
	if (overlapX < overlapY)
	{
		// comp1이 comp2의 왼쪽에 있다면
		if (rect1.left < rect2.left)
			info.collisionNormal = Vector(-1, 0); // 오른쪽면 충돌
		else
			info.collisionNormal = Vector(1, 0); // 왼쪽면 충돌
	}
	else // Y축 겹침이 X축 겹침보다 작다면, 수평 충돌
	{
		// comp1이 comp2의 위에 있다면
		if (rect1.top < rect2.top)
			info.collisionNormal = Vector(0, -1); // 윗면 충돌
		else
			info.collisionNormal = Vector(0, 1); // 아랫면 충돌
	}

	return info;
}

void CollisionManager::Destroy()
{
	collisionComponents.clear();
}

void CollisionManager::RemovePendingKillComponents()
{
	auto it = collisionComponents.begin();
	while (it != collisionComponents.end())
	{
		if ((*it)->IsPendingKill())
		{
			CollisionComponent* comp = *it;

			auto pairIt = collisionPairs.begin();
			while (pairIt != collisionPairs.end())
			{
				if (pairIt->first.first == comp || pairIt->first.second == comp)
				{
					lastCollisionInfo.erase(pairIt->first);
					pairIt = collisionPairs.erase(pairIt);
				}
				else
					++pairIt;
			}
			it = collisionComponents.erase(it);
		}
		else
			++it;
	}
}

void CollisionManager::RegisterCollisionComponent(CollisionComponent* comp)
{
	if (!comp) return;

	collisionComponents.push_back(comp);
}

void CollisionManager::UnregisterCollisionComponent(CollisionComponent* comp)
{
	if (!comp) return;

	comp->SetPendginKill(true);
}
