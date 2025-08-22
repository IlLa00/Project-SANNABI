#include "pch.h"
#include "Turret.h"
#include "SpriteRenderComponent.h"
#include "CollisionComponent.h"
#include "BulletPool.h"
#include "Bullet.h"
#include "CameraManager.h"

void Turret::Init()
{
	Super::Init();

	// 포지션은 타일맵에서.. 일단 임의로 하자
	position = Vector(1707,347);

	bodyRenderComponent = new SpriteRenderComponent;
	bodyRenderComponent->Init(this);
	bodyRenderComponent->AddAnimation("Idle", "ENE_TurretBody_Idle", 9, 1.f);
	bodyRenderComponent->AddAnimation("Aim", "ENE_TurretBody_Aim", 11, 1.f);
	bodyRenderComponent->AddAnimation("Shoot", "ENE_TurretBody_Shoot", 18, 1.f);

	bodyRenderComponent->AddAnimation("Explosion", "ENE_ExplosionGeneral_Middle", 12, 2.f);
	AddComponent(bodyRenderComponent);

	gunRenderComponent = new SpriteRenderComponent;
	gunRenderComponent->Init(this);
	gunRenderComponent->AddAnimation("GunIdle", "ENE_TurretGun_Idle", 9, 1.f);
	gunRenderComponent->AddAnimation("GunAim", "ENE_TurretGun_Aim", 11, 1.f);
	gunRenderComponent->AddAnimation("GunShoot", "ENE_TurretGun_Shoot", 18, 1.f);
	AddComponent(gunRenderComponent);

	poolInstance = BulletPool::GetInstance();
	if (poolInstance)
	{
		poolInstance->CreatePool(20);
		currentBullets.reserve(20);
	}
}

void Turret::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!bDamaged)
	{
		// 상태변화.
		if (bDetectTarget) // 감지가 되면
		{
			if (turretState == ETurretState::Idle)
			{
				SetTurretState(ETurretState::Aiming);
				elapsedTime = 0.f;
			}
			else if (turretState == ETurretState::Aiming)
			{
				elapsedTime += deltaTime;
				if (elapsedTime >= shootCooldown)
				{
					SetTurretState(ETurretState::Shooting);
					elapsedTime = 0.f;
				}
			}
			else if (turretState == ETurretState::Shooting)
			{
				elapsedTime += deltaTime;
				if (elapsedTime >= resetCooldown)
				{
					SetTurretState(ETurretState::Idle);
					elapsedTime = 0.f;
				}
			}
		}
		else
		{
			if (!bShooting) // 쏘고있을 때, 감지범위를 벗어나면 바로 Idle되는게 아닌 쏠건 다 쏘기 
			{
				SetTurretState(ETurretState::Idle);
				elapsedTime = 0.f;
			}
		}

		switch (turretState)
		{
		case ETurretState::Idle:
			UpdateIdle(deltaTime);
			break;
		case ETurretState::Aiming:
			UpdateAiming(deltaTime);
			break;
		case ETurretState::Shooting:
			UpdateShooting(deltaTime);
			break;
		}

		for (auto it = currentBullets.begin(); it != currentBullets.end();)
		{
			Bullet* bullet = *it;
			if (!bullet)
			{
				it = currentBullets.erase(it);
				continue;
			}

			bool shouldRemove = false;

			// 화면 밖 체크
			if (bullet->GetPosition().x < (CameraManager::GetInstance()->GetCameraPos().x - GWinSizeX / 2) - 200 ||
				bullet->GetPosition().x >(CameraManager::GetInstance()->GetCameraPos().x + GWinSizeX / 2) + 200 ||
				bullet->GetPosition().y < (CameraManager::GetInstance()->GetCameraPos().y - GWinSizeY / 2) - 200 ||
				bullet->GetPosition().y >(CameraManager::GetInstance()->GetCameraPos().y + GWinSizeY / 2) + 200)
			{
				shouldRemove = true;
			}
			else if (!bullet->IsActive())
			{
				shouldRemove = true;
			}

			if (shouldRemove)
			{
				poolInstance->ReturnProjectile(bullet);
				it = currentBullets.erase(it);
			}
			else
			{
				bullet->Update(deltaTime);
				++it;
			}
		}
	}
	else
	{
		elapsedTimeExplosion += deltaTime;

		if (elapsedTimeExplosion >= explosionDuration)
		{
			SetActive(false);
			Destroy();
		}
		else
		{
			bodyRenderComponent->PlayAnimation("Explosion");

			if(gunRenderComponent->IsActive())
				gunRenderComponent->SetActive(false);

			if (collisionComponent->IsActive())
				collisionComponent->SetActive(false);

			if (perceiveComponent->IsActive())
				perceiveComponent->SetActive(false);
		}
	}
}

void Turret::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	for (auto& bullet : currentBullets)
	{
		if (!bullet || !bullet->IsActive()) continue;

		bullet->Render(_hdcBack);
	}

	DebugRender(_hdcBack);
}

void Turret::Destroy()
{
	for (auto& bullet : currentBullets)
		poolInstance->ReturnProjectile(bullet);

	currentBullets.clear();
	poolInstance = nullptr;
	
	Super::Destroy();
}

void Turret::TakeDamage()
{
	Super::TakeDamage();

	// 애니메이션 재생
	bDamaged = true;
	// 폭발
}

void Turret::UpdateIdle(float deltaTime)
{
	// 여긴 애니메이션이 필요없을듯.
	bShooting = false;
}

void Turret::UpdateAiming(float deltaTime)
{
	bShooting = false;

	gunDirection = target->GetPosition() - position;
	gunDirection.Normalize();
}

void Turret::UpdateShooting(float deltaTime)
{
	if (!poolInstance) return;
	// 발사!!
	bShooting = true;

	bulletElapsedTime += deltaTime;

	if (bulletElapsedTime >= 0.25)
	{
		Bullet* curProjectile = poolInstance->GetProjectile(GetPosition(), gunDirection, bulletSpeed);
		if (curProjectile)
		{
			curProjectile->SetOwner(this);
			currentBullets.push_back(curProjectile);
		}
		bulletElapsedTime = 0.f;
	}
}

void Turret::OnCharacterBeginOverlap(CollisionComponent* other, HitResult info)
{
	Super::OnCharacterBeginOverlap(other, info);

}

void Turret::OnCharacterEndOverlap(CollisionComponent* other, HitResult info)
{
	Super::OnCharacterEndOverlap(other, info);
	// 필요한가?
}

void Turret::OnPerceiveCharacter(CollisionComponent* other, HitResult info)
{
	Super::OnPerceiveCharacter(other, info);
	
}

void Turret::OffPerceiveCharacter(CollisionComponent* other, HitResult info)
{
	Super::OffPerceiveCharacter(other, info);
}

void Turret::SetTurretState(ETurretState newState)
{
	turretState = newState;

	switch (turretState)
	{
	case ETurretState::Idle:
		bodyRenderComponent->PlayAnimation("Idle");
		gunRenderComponent->PlayAnimation("GunIdle");
		break;
	case ETurretState::Aiming:
		bodyRenderComponent->PlayAnimation("Aim");
		gunRenderComponent->PlayAnimation("GunAim");
		break;
	case ETurretState::Shooting:
		bodyRenderComponent->PlayAnimation("Shoot");
		gunRenderComponent->PlayAnimation("GunShoot");
		break;
	}
}

void Turret::DebugRender(HDC _hdcBack)
{
	wstring stateStr = GetTurretStateString(turretState);

	wstring outputStr = L"TurretState(" + stateStr + L")";
	::TextOut(_hdcBack, 25, 400, outputStr.c_str(), static_cast<int32>(outputStr.size()));
}

const wchar_t* Turret::GetTurretStateString(ETurretState state)
{
	switch (state)
	{
	case ETurretState::Idle:
		return L"Idle";
	case ETurretState::Aiming:
		return L"Aiming";
	case ETurretState::Shooting:
		return L"Shooting";
	default:
		return L"Unknown";
	}
}


