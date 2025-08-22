#include "pch.h"
#include "Enemy.h"
#include "CollisionComponent.h"
#include "Player.h"

void Enemy::Init()
{
	Super::Init();

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(100, 100);
	collisionComponent->SetCollisionChannel(ECollisionChannel::Character); // 정안되면 Enemy채널만들기..
	collisionComponent->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
		{
			if (other->GetOwner() == this) return;

			OnCharacterBeginOverlap(other, info); // 맞으면 실행
		};
	collisionComponent->OnComponentEndOverlap = [this](CollisionComponent* other, HitResult info)
		{
			OnCharacterEndOverlap(other, info);
		};
	AddComponent(collisionComponent);
	
	perceiveComponent = new CollisionComponent;
	perceiveComponent->Init(this);
	perceiveComponent->SetCollisionSize(dectionRange.x, dectionRange.y); // 임의
	perceiveComponent->SetCollisionChannel(ECollisionChannel::Perception); // 정안되면 Enemy채널만들기..
	perceiveComponent->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
		{
			if (other->GetOwner() == this) return;

			OnPerceiveCharacter(other, info);
		};
	perceiveComponent->OnComponentEndOverlap = [this](CollisionComponent* other, HitResult info)
		{
			OffPerceiveCharacter(other, info);
		};
	AddComponent(perceiveComponent);
}

void Enemy::Update(float deltaTime)
{
	Super::Update(deltaTime);

	// if(bDamaged)
		// 죽기.
}

void Enemy::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);
}

void Enemy::Destroy()
{
	target = nullptr;

	Super::Destroy();
}

void Enemy::TakeDamage()
{
	Super::TakeDamage();


}

void Enemy::OnCharacterBeginOverlap(CollisionComponent* other, HitResult info)
{
	// 상태바꾸기?
	bDamaged = true;
}

void Enemy::OnCharacterEndOverlap(CollisionComponent* other, HitResult info)
{
	// 상태바꾸기?
}

void Enemy::OnPerceiveCharacter(CollisionComponent* other, HitResult info)
{
	bDetectTarget = true;

	target = other->GetOwner();
}

void Enemy::OffPerceiveCharacter(CollisionComponent* other, HitResult info)
{
	bDetectTarget = false;
}
