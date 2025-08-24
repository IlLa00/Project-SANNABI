#pragma once

#include "Actor.h"
#include "PhysicsComponent.h"

// 상태에 따른 애니메이션 재생 
enum class EPlayerMovementState
{
	Idle, // 가만히 있음
	Run, // 달림
	Jump, // 위방향 공중
	Fall, // 아래방향 공중
	Land, // 착륙  인데 빼도됨
};

enum class EPlayerActionState
{
	None,
	Jump, // 위방향 공중

	GrappleFire, // 사슬 발사
	GrappleSwing, // 사슬 스윙
	GrappleReelIn, // 사슬 감기
	
	Ceiling, // 천장
	WallGrab, // 벽타기

	CharageDashStart, // 차지 공격 준비
	ChargeReady, // 차지 완
	ChargeAttack, // 차지 공격

	DashAttack, // 사슬팔 대쉬 공격

	AirDash,

	TakeDamage, // 피격

	Die // 죽음
};

class SpriteRenderComponent;
class GrapplingComponent;
class GrapplingHookProjectile;
class CollisionComponent;
class PhysicsComponent;
class TextureResource;

class Player : public Actor
{
	using Super = Actor;

public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void OnCharacterBeginOverlap(CollisionComponent* other, HitResult info) override;

	void SetDelayedFunction(void (Player::* func1)(), void (Player::* func2)(), float delay1 = 1.0f, float delay2 = 1.0f);

	void NoneInput();
	void OnPressA();
	void OnPressD();
	void OnPressW();
	void OnPressS();
	void OnSpaceBarDown();
	void OnMouseDown();
	void OnMouseUp(); 
	void OnShiftDown();
	void OnShiftUp();

	void OnGrappling(GrapplingHookProjectile* curprojectile);
	void OffGrappling();

	void Dash();
	void EndDash();
	void Attack();

	void UpdateMovementState(EPlayerMovementState state);
	void UpdateActionState(EPlayerActionState state);

	void SetTarget(Actor* newTarget) { target = newTarget; }
	Actor* GetTarget() { return target; }

	void UpdateAnimation();

	EPlayerMovementState GetMovementState() { return movementState; }

	void SetActionState(EPlayerActionState newState) { actionState = newState; }
	EPlayerActionState GetActionState() { return actionState; }

	SpriteRenderComponent* GetArmRenderComponent() { return armRenderComponent; }


private:
	void OnDelayedControlRecovery();
	void OnDelayedCollisionRecovery();

	const wchar_t* GetMovementStateString(EPlayerMovementState state);
	const wchar_t* GetActionStateString(EPlayerActionState state);
	const wchar_t* GetPhysicsStateString(EPhysicsState state);
	const wchar_t* GetBoolString(bool value) { return value ? L"true" : L"false"; }

	void DebugRender(HDC _hdcBack);

private:
	SpriteRenderComponent* bodyRenderComponent = nullptr;
	SpriteRenderComponent* armRenderComponent = nullptr;
	TextureResource* chainLinkTexture = nullptr;

	GrapplingComponent* grapplingComponent = nullptr;
	CollisionComponent* collisionComponent = nullptr;
	PhysicsComponent* physicsComponent = nullptr;  

	EPlayerMovementState movementState;
	EPlayerActionState actionState;

	Actor* target = nullptr;
	vector<CollisionComponent*> detectComponents;

	float chargingTimer = 0.f;
	float chargeTime = 1.f;
	float maxChargeTime = 5.f;
	bool bHasDetected = false;

	bool bAttacking = false;
	bool bDamaging = false;

	bool showAimingLine = true;  
	HPEN chainPen;

	float delayTimer1 = 0.0f;
	float delayTimer2 = 0.0f;
	bool bHasDelayedFunction = false;
	void (Player::* delayedFunctionPtr1)() = nullptr; 
	void (Player::* delayedFunctionPtr2)() = nullptr;  
};

