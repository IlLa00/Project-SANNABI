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

	// 화면 밖으로 나가면 풀에게 돌려주기
	if (projectilePos.x < CameraManager::GetInstance()->GetCameraPos().x - GWinSizeX / 2 ||
		projectilePos.x > CameraManager::GetInstance()->GetCameraPos().x + GWinSizeX / 2 ||
		projectilePos.y < CameraManager::GetInstance()->GetCameraPos().y - GWinSizeY / 2 ||
		projectilePos.y > CameraManager::GetInstance()->GetCameraPos().y + GWinSizeY / 2
		)
		OffGrappling();

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
			curProjectile->SetOwner(owner);

			CollisionComponent* comp = curProjectile->GetComponent<CollisionComponent>();
			if (comp)
			{
				comp->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult result)
					{
						if (other && other->GetOwner() == owner)
							return;
						
						curProjectile->SetFlying(false);
						OnGrappling();
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
	GrapplingHookProjectilePool::GetInstance()->ReturnProjectile(curProjectile);
	curProjectile = nullptr; // 주의!
	bFiring = false;
}

