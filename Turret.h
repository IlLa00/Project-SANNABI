#pragma once

#include "Enemy.h"
#include "CollisionManager.h"

class BulletPool;
class Bullet;
class SpriteRenderComponent;

enum class ETurretState
{
	Idle,
	Aiming,
	Shooting
};

class Turret : public Enemy
{
	using Super = Enemy;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void TakeDamage() override;

	void OnCharacterBeginOverlap(CollisionComponent* other, HitResult info) override;
	void OnCharacterEndOverlap(CollisionComponent* other, HitResult info) override;
	void OnPerceiveCharacter(CollisionComponent* other, HitResult info) override;
	void OffPerceiveCharacter(CollisionComponent* other, HitResult info) override;

	void UpdateIdle(float deltaTime);
	void UpdateAiming(float deltaTime);
	void UpdateShooting(float deltaTime);

	void SetTurretState(ETurretState newState);

private:
	void DebugRender(HDC _hdcBack);
	const wchar_t* GetTurretStateString(ETurretState state);

private:
	SpriteRenderComponent* bodyRenderComponent;
	SpriteRenderComponent* gunRenderComponent;

	ETurretState turretState = ETurretState::Idle;

	float elapsedTime = 0.f;
	float shootCooldown = 3.f; // 에이밍 -> 슛
	float resetCooldown = 5.f; // 대기 -> 에이밍
	
	bool bShooting = false;

	Vector gunDirection = Vector(0, 0);

	BulletPool* poolInstance = nullptr;
	vector<Bullet*> currentBullets;

	float bulletSpeed = 100.f;
	float bulletElapsedTime = 0.f;

	bool bDamaged = false;

	float explosionDuration = 2.f;
	float elapsedTimeExplosion = 0.f;
};

