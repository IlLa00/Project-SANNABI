#include "pch.h"
#include "GrapplingComponent.h"
#include "GrapplingHookProjectilePool.h"
#include "GrapplingHookProjectile.h"
#include "CollisionComponent.h"
#include "Actor.h"
#include "Player.h"
#include "CameraManager.h"

void GrapplingComponent::Init(Actor* _owner)
{
	Super::Init(_owner);

	poolInstance = GrapplingHookProjectilePool::GetInstance();
	if (poolInstance)
		poolInstance->CreatePool(10);
}

void GrapplingComponent::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!curProjectile) return;

	curProjectile->Update(deltaTime);

	Vector projectilePos = curProjectile->GetPosition();

	if (!bFiring) return;

	// 사슬팔의 최대길이보다 길면 돌려주기
	Vector player_to_projectile = projectilePos - owner->GetPosition();
	if (player_to_projectile.Length() > fireChainLength)
		OffGrappling();
}

void GrapplingComponent::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (curProjectile)
		curProjectile->Render(_hdcBack);
}

void GrapplingComponent::Destroy()
{
	Super::Destroy();
}

void GrapplingComponent::FireGrapple(Vector direction)
{
	if (poolInstance && !curProjectile) // 발사!
	{
		curProjectile = poolInstance->GetProjectile(owner->GetPosition(), direction, pullSpeed);
		if (curProjectile)
		{
			bFiring = true;
			curProjectile->SetActive(true);
			curProjectile->SetOwner(owner);

			CollisionComponent* comp = curProjectile->GetComponent<CollisionComponent>();
			if (comp)
			{
				comp->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult result)
					{
						if (!curProjectile) return;

						if (other && other->GetOwner() == owner)
							return;
						
						curProjectile->SetFlying(false);

						// 여기서 지면 천장 벽 등등일지.. 캐릭터일지!
						if(other->GetCollisionChannel() == ECollisionChannel::WorldStatic ||
							other->GetCollisionChannel() == ECollisionChannel::WorldDynamic)
							OnGrappling();
						else if (other->GetCollisionChannel() == ECollisionChannel::Character)
						{
							bFiring = false;

							Player* player = dynamic_cast<Player*>(owner);
							if (player)
							{
								player->SetTarget(other->GetOwner());
								player->Dash();
							}

							GrapplingHookProjectilePool::GetInstance()->ReturnProjectile(curProjectile);
							curProjectile = nullptr; // 주의!
						}
					};
			}
		}
	}
}

void GrapplingComponent::OnGrappling()
{
	Player* player = dynamic_cast<Player*>(owner);
	if (player)
		player->OnGrappling(curProjectile->GetPosition());
}

void GrapplingComponent::OffGrappling()
{
	if (!curProjectile) return;

	GrapplingHookProjectilePool::GetInstance()->ReturnProjectile(curProjectile);
	curProjectile = nullptr; // 주의!
	bFiring = false;
}

