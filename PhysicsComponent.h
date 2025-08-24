#pragma once

#include "Component.h"
#include "CollisionManager.h"

class CollisionComponent;
class GrapplingHookProjectile;

enum class EPhysicsState
{
	Normal,
	Grappling,
	ExtendingChain,
	RightWallClimbing,
	LeftWallClimbing,
	CeilingHang,
	Dashing
};

class PhysicsComponent : public Component
{
	using Super = Component;

public:
	void Init(Actor* _owner) override;
	void Update(float deltaTime) override;
	void Destroy() override;

	void UpdateNormalPhysics(float deltaTime); // 기본 물리 업데이트
	void UpdateGrapplePhysics(float deltaTime); // 사슬팔 물리 업데이트
	void UpdateExtendChainPhysics(float deltaTime);
	void UpdateCeilingPhysics(float deltaTime);
	void UpdateClimbingPhysics(float deltaTime);
	void UpdateDash(float deltaTime);

	void Idle();
	void Move();
	void Jump();
	void ReadyForDash();
	void ExtendChain();

	void DashToPosition(Vector position);
	void EndDash();

	void AirDash();
	void KnockBack(Vector collisionNormal);

	bool CanJump() { return bOnGround && !bJumping; }
	bool IsOnGround() { return bOnGround; }
	bool IsOnCeiling() { return bOverlapCeiling; }
	bool IsOnLeftWall() { return bOverlapLeftWall; }
	bool IsOnRightWall() { return bOverlapRightWall; }

	bool IsGrappling() { return physicsState == EPhysicsState::Grappling; }

	void OnGroundBeginOverlap(CollisionComponent* other, HitResult info);
	void OnGroundEndOverlap(CollisionComponent* other, HitResult info);

	void StartGrappling(GrapplingHookProjectile* projectile);
	void EndGrappling();

	void SetPhysicsState(EPhysicsState newState);
	EPhysicsState GetPhysicsState() { return physicsState;}

	/*void SetGrapplePoint(Vector newPoint) { grapplePoint = newPoint; }
	Vector GetGrapplePoint() { return grapplePoint; }*/

	GrapplingHookProjectile* GetCurrentProjectile() { return curPrjoectile;}

	float GetDashCurve(float t);
	bool IsDashing() const { return bIsDash; }

	bool IsJustRelaeasedGrapple() { return bJustReleasedGrapple; }

private:
	float gravity = 1200.f;
	float jumpForce = 500.f;  // 점프 힘
	float maxFallSpeed = 1000.f;  // 최대 낙하 속도
	float swingInputForce = 250.0f;    // 입력 가속력 (큰 값)

	float swingDamping = 0.998f;
	float swingGravityMult = 2.0f;
	float timingBonusMult = 1.5f;    // 타이밍 보너스 배수

	EPhysicsState physicsState = EPhysicsState::Normal;
	bool bOnGround = false;
	bool bJumping = false;
	bool bFalling = false;

	GrapplingHookProjectile* curPrjoectile = nullptr;
	Vector lastProjectilePosition;
	float grappleLength = 10.0f;
	float angularVelocity = 0.0f; // 각속도

	float currentAngle = 0.0f;
	Vector toHook;

	bool bJustReleasedGrapple = false;  // 멤버 변수 추가
	float grappleReleaseTimer = 0.0f;   // 타이머 추가
	const float GRAPPLE_RELEASE_TIME = 0.5f; // 0.5초간 보호

	bool bExtendingChain = false;        // 체인 확장 중인지
	float chainExtendSpeed = 2000.0f;     // 체인 확장 속도
	
	bool bRolling = false;

	bool bIsDash = false;
	Vector dashStartPostion;
	Vector dashTargetPostion;
	float dashCurrentTime = 0.f;
	float dashTotalTime = 0.5f;
	float dashSpeed = 1000.f;


	// 이것들은 충돌여부를 나타냄, 실제 피직스 상태를 나타내는게 아님
	bool bOverlapCeiling = false; 
	bool bOverlapLeftWall = false;
	bool bOverlapRightWall = false;

	bool bBlockedLeft = false;
	bool bBlockedRight = false;
};

