#include "pch.h"
#include "Bullet.h"
#include "CollisionComponent.h"
#include "BulletPool.h"
#include "CollisionManager.h"

void Bullet::Init()
{
	Super::Init();

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetActive(false);
	collisionComponent->SetCollisionSize(5, 5);
	collisionComponent->SetCollisionChannel(ECollisionChannel::Projectile);
	
	AddComponent(collisionComponent);
}

void Bullet::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!bActive) return; // 풀에서 나온 객체인가?

	Vector currentPos = GetPosition();
	Vector newPos = currentPos + (direction * speed * deltaTime);
	SetPosition(newPos);
}

void Bullet::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (!bActive) return;
}

void Bullet::Activate(Vector _position, Vector _direction, float _speed)
{
	bActive = true;

	collisionComponent->SetActive(true);

	direction = _direction;
	speed = _speed;
	SetPosition(_position);
}

void Bullet::Deactivate()
{
	bActive = false;

	direction = Vector(0, 0);
	speed = 0.f;
	SetPosition(Vector(-100, -100));
}