#include "pch.h"
#include "CollisionManager.h"
#include "Player.h"
#include "GrapplingHookProjectile.h"
#include "CollisionComponent.h"
#include "TimerManager.h"

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
	bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::DeathTile] = 1;

	bBlock[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::Projectile] = 1;
	bBlock[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::Character] = 1;
	bBlock[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::CharacterInvincible] = 1;

	bBlock[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::Projectile] = 1;
	bBlock[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::Character] = 1;
	bBlock[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::CharacterInvincible] = 1;

	bBlock[(int)ECollisionChannel::CharacterInvincible][(int)ECollisionChannel::Perception] = 1;
	bBlock[(int)ECollisionChannel::CharacterInvincible][(int)ECollisionChannel::WorldDynamic] = 1;
	bBlock[(int)ECollisionChannel::CharacterInvincible][(int)ECollisionChannel::WorldStatic] = 1;

	bBlock[(int)ECollisionChannel::Perception][(int)ECollisionChannel::Character] = 1;
	bBlock[(int)ECollisionChannel::Perception][(int)ECollisionChannel::CharacterInvincible] = 1;

	bBlock[(int)ECollisionChannel::DeathTile][(int)ECollisionChannel::Character] = 1;

	bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Projectile] = 1;
	bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::CharacterInvincible] = 1;
	bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::DeathTile] = 1;

	bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldStatic] = 1;
	bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldDynamic] = 1;

	bIgnore[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::WorldDynamic] = 1;
	bIgnore[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::WorldStatic] = 1;

	bIgnore[(int)ECollisionChannel::CharacterInvincible][(int)ECollisionChannel::Projectile] = 1;

	bIgnore[(int)ECollisionChannel::DeathTile][(int)ECollisionChannel::Projectile] = 1;
}

void CollisionManager::Update()
{
	// 먼저 예측 충돌 처리
	ProcessPredictiveCollisions();

	// 그 다음 일반 충돌 처리
	for (int i = 0; i < collisionComponents.size(); i++)
	{
		if (!collisionComponents[i]->IsActive()) continue;

		for (int j = i + 1; j < collisionComponents.size(); j++)
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

void CollisionManager::ProcessPredictiveCollisions()
{
	const float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	for (auto* movingComp : collisionComponents)
	{
		if (!movingComp || !movingComp->IsActive())
			continue;

		// 움직이는 객체만 체크 
		ECollisionChannel movingChannel = movingComp->GetCollisionChannel();
		if (movingChannel != ECollisionChannel::Character &&
			movingChannel != ECollisionChannel::CharacterInvincible &&
			movingChannel != ECollisionChannel::Projectile)
			continue;

		Vector currentVelocity = GetComponentVelocity(movingComp);
		if (currentVelocity.x == 0 && currentVelocity.y == 0)
			continue; 

		// 다음 프레임 예측 위치 계산
		Vector predictedPos = movingComp->GetOwner()->GetPosition();
		predictedPos.x += currentVelocity.x * deltaTime;
		predictedPos.y += currentVelocity.y * deltaTime;

		// 예측 위치에서의 충돌 체크
		for (auto* staticComp : collisionComponents)
		{
			if (!staticComp || !staticComp->IsActive() || staticComp == movingComp)
				continue;

			ECollisionChannel staticChannel = staticComp->GetCollisionChannel();

			if (staticChannel == ECollisionChannel::Perception)
				continue;

			if (movingChannel == ECollisionChannel::Projectile &&
				staticChannel == ECollisionChannel::Projectile)
				continue;

			if (movingChannel == ECollisionChannel::Character &&
				staticChannel == ECollisionChannel::Character)
				continue;

			if (bIgnore[(int)movingChannel][(int)staticChannel] ||
				!bBlock[(int)movingChannel][(int)staticChannel])
				continue;

			// 예측 충돌 검사
			HitResult predictedHit = CheckPredictiveCollision(movingComp, staticComp, predictedPos);

			if (predictedHit.isColliding)
			{
				// 충돌이 예측되면 위치 보정
				CorrectPositionBeforeCollision(movingComp, staticComp, predictedHit, currentVelocity);
			}
		}
	}
}

HitResult CollisionManager::CheckPredictiveCollision(CollisionComponent* movingComp,
	CollisionComponent* staticComp, Vector predictedPos)
{
	HitResult info;

	if (!movingComp || !staticComp) return info;

	// 움직이는 컴포넌트의 예측 바운딩 박스 계산
	RECT movingRect = movingComp->GetBoundingBox();
	Vector currentPos = movingComp->GetOwner()->GetPosition();
	float offsetX = predictedPos.x - currentPos.x;
	float offsetY = predictedPos.y - currentPos.y;

	RECT predictedRect;
	predictedRect.left = movingRect.left + offsetX;
	predictedRect.right = movingRect.right + offsetX;
	predictedRect.top = movingRect.top + offsetY;
	predictedRect.bottom = movingRect.bottom + offsetY;

	// 정적 컴포넌트의 바운딩 박스
	RECT staticRect = staticComp->GetBoundingBox();

	// AABB 충돌 체크
	if (predictedRect.right < staticRect.left || predictedRect.left > staticRect.right ||
		predictedRect.bottom < staticRect.top || predictedRect.top > staticRect.bottom)
		return info;

	info.isColliding = true;

	// 충돌 정보 계산
	float overlapX = min(predictedRect.right, staticRect.right) - max(predictedRect.left, staticRect.left);
	float overlapY = min(predictedRect.bottom, staticRect.bottom) - max(predictedRect.top, staticRect.top);

	if (overlapX < overlapY)
	{
		if (predictedRect.left < staticRect.left)
		{
			info.collisionNormal = Vector(-1, 0);
			info.collisionPoint = Vector(predictedRect.right, (predictedRect.top + predictedRect.bottom) / 2.0f);
			info.penetrationDepth = overlapX;
		}
		else
		{
			info.collisionNormal = Vector(1, 0);
			info.collisionPoint = Vector(predictedRect.left, (predictedRect.top + predictedRect.bottom) / 2.0f);
			info.penetrationDepth = overlapX;
		}
	}
	else
	{
		if (predictedRect.top < staticRect.top)
		{
			info.collisionNormal = Vector(0, -1);
			info.collisionPoint = Vector((predictedRect.left + predictedRect.right) / 2.0f, predictedRect.bottom);
			info.penetrationDepth = overlapY;
		}
		else
		{
			info.collisionNormal = Vector(0, 1);
			info.collisionPoint = Vector((predictedRect.left + predictedRect.right) / 2.0f, predictedRect.top);
			info.penetrationDepth = overlapY;
		}
	}

	return info;
}

void CollisionManager::CorrectPositionBeforeCollision(CollisionComponent* movingComp,
	CollisionComponent* staticComp, HitResult& predictedHit, Vector& velocity)
{
	if (!movingComp || !movingComp->GetOwner()) return;

	Vector currentPos = movingComp->GetOwner()->GetPosition();

	if (abs(predictedHit.collisionNormal.x) > 0.5f) // 수평 충돌
	{
		velocity.x = 0;

		RECT staticRect = staticComp->GetBoundingBox();
		RECT movingRect = movingComp->GetBoundingBox();
		float halfWidth = (movingRect.right - movingRect.left) / 2.0f;
		float centerY = currentPos.y; 

		if (predictedHit.collisionNormal.x > 0) 
		{
			float newX = (float)staticRect.right + halfWidth;
			movingComp->GetOwner()->SetPosition(Vector(newX, centerY));
		}
		else 
		{
			float newX = (float)staticRect.left - halfWidth;
			movingComp->GetOwner()->SetPosition(Vector(newX, centerY));
		}
	}
	else 
	{
		velocity.y = 0;

		RECT staticRect = staticComp->GetBoundingBox();
		RECT movingRect = movingComp->GetBoundingBox();
		float halfHeight = (movingRect.bottom - movingRect.top) / 2.0f;
		float centerX = currentPos.x; 

		if (predictedHit.collisionNormal.y > 0) 
		{
			float newY = (float)staticRect.bottom + halfHeight;
			movingComp->GetOwner()->SetPosition(Vector(centerX, newY));
		}
		else 
		{
			float newY = (float)staticRect.top - halfHeight;
			movingComp->GetOwner()->SetPosition(Vector(centerX, newY));
		}
	}

	if (movingComp->GetOwner())
		movingComp->GetOwner()->SetVelocity(velocity);
}

Vector CollisionManager::GetComponentVelocity(CollisionComponent* comp)
{
	if (!comp || !comp->GetOwner())
		return Vector(0, 0);

	return comp->GetOwner()->GetVelocity();
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

	if (overlapX < overlapY)
	{
		if (rect1.left < rect2.left)
		{
			info.collisionNormal = Vector(-1, 0); 
			info.collisionPoint = Vector(rect1.right, (rect1.top + rect1.bottom) / 2.0f);
		}
		else
		{
			info.collisionNormal = Vector(1, 0); 
			info.collisionPoint = Vector(rect1.left, (rect1.top + rect1.bottom) / 2.0f);
		}
	}
	else 
	{
		if (rect1.top < rect2.top)
		{
			info.collisionNormal = Vector(0, -1);
			info.collisionPoint = Vector((rect1.left + rect1.right) / 2.0f, rect1.bottom);
		}
		else
		{
			info.collisionNormal = Vector(0, 1);
			info.collisionPoint = Vector((rect1.left + rect1.right) / 2.0f, rect1.top); 
		}
	}

	return info;
}

vector<CollisionComponent*> CollisionManager::DetectEnemiesInRange(Vector playerPos, float radius, CollisionComponent* playerComp)
{
	vector<CollisionComponent*> detectedEnemies;

	float radiusSquared = radius * radius;

	for (auto* comp : collisionComponents)
	{
		if (!comp || !comp->IsActive() || comp == playerComp)
			continue;

		if (comp->GetCollisionChannel() != ECollisionChannel::Character)
			continue;

		if (playerComp && comp->GetOwner() == playerComp->GetOwner())
			continue;

		if (IsComponentInCircle(comp, playerPos, radiusSquared))
			detectedEnemies.push_back(comp);
	}

	return detectedEnemies;
}

bool CollisionManager::IsComponentInCircle(CollisionComponent* comp, Vector centerPos, float radiusSquared)
{
	if (!comp || !comp->GetOwner()) return false;

	Vector compPos = comp->GetOwner()->GetPosition();

	float dx = compPos.x - centerPos.x;
	float dy = compPos.y - centerPos.y;
	float distanceSquared = dx * dx + dy * dy;

	return distanceSquared <= radiusSquared;
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