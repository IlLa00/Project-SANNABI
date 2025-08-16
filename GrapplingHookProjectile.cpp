#include "pch.h"
#include "GrapplingHookProjectile.h"
#include "CollisionComponent.h"
#include "GrapplingHookProjectilePool.h"
#include "CollisionManager.h"


void GrapplingHookProjectile::Init()
{
	Super::Init();

	collsionComponent = new CollisionComponent;
	collsionComponent->Init(this);
	collsionComponent->SetCollisionSize(5, 5);
	collsionComponent->SetCollisionChannel(ECollisionChannel::Projectile);
	AddComponent(collsionComponent);

}

void GrapplingHookProjectile::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!bActive) return; // 풀에서 나온 객체인가?
	if (!IsFlying()) return; // 날아가고 있는가?

	Vector currentPos = GetPosition();
	Vector newPos = currentPos + (direction * speed * deltaTime);
	SetPosition(newPos);
}

void GrapplingHookProjectile::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (!bActive) return;
}

void GrapplingHookProjectile::Destroy()
{
	Super::Destroy();
}

void GrapplingHookProjectile::Activate(Vector _position, Vector _direction, float _speed)
{
	bActive = true;

	direction = _direction;
	speed = _speed;
	position = _position;

	SetFlying(true);
}

void GrapplingHookProjectile::Deactivate()
{
	bActive = false;

	direction = Vector(0, 0);
	speed = 0.f;
	position = Vector(0, 0);

	SetFlying(false);

}
