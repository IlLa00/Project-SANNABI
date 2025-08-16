#include "pch.h"
#include "Turret.h"
#include "SpriteRenderComponent.h"
#include "CollisionComponent.h"
#include "BulletPool.h"
#include "Bullet.h"

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
	AddComponent(bodyRenderComponent);

	gunRenderComponent = new SpriteRenderComponent;
	gunRenderComponent->Init(this);
	gunRenderComponent->AddAnimation("GunIdle", "ENE_TurretGun_Idle", 9, 1.f);
	gunRenderComponent->AddAnimation("GunAim", "ENE_TurretGun_Aim", 11, 1.f);
	gunRenderComponent->AddAnimation("GunShoot", "ENE_TurretGun_Shoot", 18, 1.f);
	AddComponent(gunRenderComponent);

	poolInstance = BulletPool::GetInstance();
	if (poolInstance)
		poolInstance->CreatePool(20);
}

void Turret::Update(float deltaTime)
{
	Super::Update(deltaTime);

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
}

void Turret::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);
}

void Turret::Destroy()
{
	Super::Destroy();
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
			curProjectile->SetOwner(owner);

		bulletElapsedTime = 0.f;
	}
}

void Turret::OnCharacterBeginOverlap(CollisionComponent* other, HitResult info)
{
	// 애니메이션 재생?
	// 을 컴포넌트로 사용한다면, 애니메이션 재생이 끝나면 죽기처리.
	// 애니메이션 재생 도중엔 콜리전 끄기? 플레이어가 공격 다시 못하게..
	
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
