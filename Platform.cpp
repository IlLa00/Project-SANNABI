#include "pch.h"
#include "Platform.h"
#include "CollisionComponent.h"
#include "TextureResource.h"
#include "GrapplingHookProjectile.h"

void Platform::Init()
{
	Super::Init();

	texture = new TextureResource();
	texture->Load("Platform");
	
	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(texture->GetSizeX() / 2 - 50, texture->GetSizeY() / 2 - 25);
	collisionComponent->SetCollisionChannel(ECollisionChannel::WorldDynamic);
	collisionComponent->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
		{
			OnBeginOverlap(other, info);
		};
	AddComponent(collisionComponent);
}

void Platform::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (bFalling)
	{
		Vector movement = Vector(0, fallingSpeed * deltaTime);
		position = position + movement;
		if (projectile)
			projectile->SetPosition(projectile->GetPosition() + movement);
	}
	else
	{
		Vector movement = Vector(0,  (- fallingSpeed / 2) * deltaTime);
		position = position + movement;
	}

	position.y = clamp(position.y, _limitMinY, _limitMaxY);

	if (projectile)
	{
		if (!projectile->IsActive())
		{
			bFalling = false;
			projectile = nullptr;
			return;
		}
		Vector projectilePosition = projectile->GetPosition();
		projectilePosition.y = clamp(projectilePosition.y, _limitMinY, _limitMaxY);
		projectile->SetPosition(projectilePosition);
	}
}

void Platform::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	texture->Render(_hdcBack, position);
}

void Platform::Destroy()
{
	SAFE_DELETE(texture);

	Super::Destroy();
}

void Platform::SetLimitPoint(float limitMinY, float limitMaxY)
{
	_limitMinY = limitMinY;
	_limitMaxY = limitMaxY;
}

void Platform::OnBeginOverlap(CollisionComponent* other, HitResult info)
{
	if (other && other->GetCollisionChannel() == ECollisionChannel::Projectile)
	{
		Vector normal = info.collisionNormal;

		if (normal.x == 0 && normal.y == -1) // 지면
		{
			bFalling = true;
			if (dynamic_cast<GrapplingHookProjectile*>(other->GetOwner()))
				projectile = dynamic_cast<GrapplingHookProjectile*>(other->GetOwner());
		}
	}
}

