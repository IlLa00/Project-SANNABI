#include "pch.h"
#include "PhysicsComponent.h"
#include "TimerManager.h"
#include "Actor.h"
#include "Player.h"
#include "CollisionComponent.h"
#include "GrapplingComponent.h"
#include "SpriteRenderComponent.h"

void PhysicsComponent::Init(Actor* _owner)
{
	Super::Init(_owner);

	if (owner)
	{
		CollisionComponent* collisionComp = owner->GetComponent<CollisionComponent>();
		if (collisionComp)
		{

			collisionComp->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
				{
					OnGroundBeginOverlap(other, info);
				};
			collisionComp->OnComponentEndOverlap = [this](CollisionComponent* other, HitResult info)
				{
					// 사슬팔인데 플레이어와 부딪히면 처리 안함
					if (other->GetCollisionChannel() == ECollisionChannel::Projectile && other->GetOwner()->GetOwner() == owner)
						return;

					if (other->GetCollisionChannel() == ECollisionChannel::Projectile ||
						other->GetCollisionChannel() == ECollisionChannel::DeathTile
						)
						owner->OnCharacterBeginOverlap(other, info);
					else
						OnGroundEndOverlap(other, info);
				};
		}
	}
}

void PhysicsComponent::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!owner) return;

	if (bJustReleasedGrapple)
	{
		grappleReleaseTimer += deltaTime;
		if (grappleReleaseTimer >= GRAPPLE_RELEASE_TIME)
		{
			bJustReleasedGrapple = false;
			grappleReleaseTimer = 0.0f;
		}
	}

	if (bRolling) return;

	switch (physicsState)
	{
	case EPhysicsState::Normal:
		UpdateNormalPhysics(deltaTime);
		break;
	case EPhysicsState::Grappling:
		UpdateGrapplePhysics(deltaTime);
		break;
	case EPhysicsState::ExtendingChain:
		UpdateExtendChainPhysics(deltaTime);
		break;
	case EPhysicsState::CeilingHang:
		UpdateCeilingPhysics(deltaTime);
		break;
	case EPhysicsState::RightWallClimbing:
		UpdateClimbingPhysics(deltaTime);
		break;
	case EPhysicsState::LeftWallClimbing:
		UpdateClimbingPhysics(deltaTime);
		break;
	case EPhysicsState::Dashing:
		UpdateDash(deltaTime);
		break;
	}

	Vector velocity = owner->GetVelocity();
	bool velocityChanged = false;

	if (bOnGround && !bJustReleasedGrapple)
	{
		velocity.y = 0;
		velocityChanged = true;
	}
	if (bOverlapCeiling)
	{
		velocity.y = 0;
		velocityChanged = true;

		if (bJumping)
			bJumping = false;
	}
	if (bOverlapLeftWall)
	{
		velocity.x = 0;
		velocityChanged = true;
	}
	if (bOverlapRightWall)
	{
		velocity.x = 0;
		velocityChanged = true;
	}

	if (velocityChanged)
		owner->SetVelocity(velocity);
}

void PhysicsComponent::Destroy()
{
	Super::Destroy();

}

void PhysicsComponent::UpdateNormalPhysics(float deltaTime)
{
	Player* player = dynamic_cast<Player*>(owner);
	if (!player) return;

	if (player->GetDirection().x <= 0)
		player->GetArmRenderComponent()->SetPosition(owner->GetPosition() + Vector(15, 3));
	else
		player->GetArmRenderComponent()->SetPosition(owner->GetPosition() + Vector(-15, 3));

	// 중력 업데이트
	Vector acceleration = owner->GetAcceleration();

	if (!bOnGround) // 공중에 있을 때는 중력 적용
		acceleration.y = gravity;
	else  // 지면에 있을 때는 중력 제거
		acceleration.y = 0;

	owner->SetAcceleration(acceleration);

	// 속도 업데이트
	Vector velocity = owner->GetVelocity();
	velocity += acceleration * deltaTime;

	// 최대 낙하 속도 제한
	if (velocity.y > maxFallSpeed)
		velocity.y = maxFallSpeed;

	if (!bOnGround && velocity.y > 0 && !bFalling)
	{
		bFalling = true;
		bJumping = false; // 점프 상태도 해제

		if (player->GetActionState() == EPlayerActionState::AirDash)
			player->SetActionState(EPlayerActionState::None);

		player->UpdateMovementState(EPlayerMovementState::Fall);
	}
	// 착지했을 때 Fall 상태 종료
	else if (bOnGround && bFalling)
	{
		bFalling = false;
		bJumping = false;
	}

	owner->SetVelocity(velocity);

	// 위치 업데이트
	Vector currentPos = owner->GetPosition();
	Vector newPos = currentPos + velocity * deltaTime;
	owner->SetPosition(newPos);

	// 점프 상태 체크
	if (bJumping && velocity.y >= 0)
		bJumping = false;
}

void PhysicsComponent::UpdateGrapplePhysics(float deltaTime)
{
	if (!owner) return;

	Player* player = dynamic_cast<Player*>(owner);
	if (!player) return;

	player->GetArmRenderComponent()->SetPosition(player->GetPosition() + Vector(0, -20));

	Vector currentPos = owner->GetPosition();
 	Vector toHookCurrent = currentPos - grapplePoint;
	float currentDistance = toHookCurrent.Length();

	GrapplingComponent* grappleComponent = owner->GetComponent<GrapplingComponent>();
	if (!grappleComponent) return;

	float maxChainLength = grappleComponent->GetMaxChainLength();
	if (currentDistance > maxChainLength)
	{
		grappleLength = maxChainLength;

		// 당기는 힘 계산
		float excessDistance = currentDistance - maxChainLength;
		Vector pullDirection = (grapplePoint - currentPos).Normalized();

		// 거리에 비례한 당기는 힘 (스프링 처럼)
		float pullStrength = 800.0f; // 조절 가능한 당기는 힘
		Vector pullForce = pullDirection * pullStrength * excessDistance;

		// 현재 속도 가져오기
		Vector currentVelocity = owner->GetVelocity();

		// 당기는 힘을 속도에 추가 (가속도 형태로)
		currentVelocity += pullForce * deltaTime;

		// 공기 저항/마찰 적용 (너무 빨라지지 않도록)
		float dampingFactor = 0.98f;
		currentVelocity *= dampingFactor;

		owner->SetVelocity(currentVelocity);
		
		Vector newPosition = currentPos + currentVelocity * deltaTime;
		owner->SetPosition(newPosition);

		return;
	}

	grappleLength = currentDistance;

	currentAngle = atan2f(toHookCurrent.y, toHookCurrent.x);

	// 각가속도 계산 (진자 운동의 기본 공식)
	float angularAccel = (gravity / grappleLength) * cosf(currentAngle) * swingGravityMult;

	swingInputForce -= 50.0f;
	swingInputForce = clamp(swingInputForce, 0.f, 250.f);

	Vector direction = owner->GetDirection();
	if (direction.x != 0)
	{
		angularAccel += -direction.x * (swingInputForce / grappleLength);

		if ((direction.x > 0 && angularVelocity > 0) ||
			(direction.x < 0 && angularVelocity < 0))
		{
			angularAccel *= timingBonusMult;
		}
	}

	// 진자 운동 관성 추가 (자연스러운 스윙감)
	if (abs(angularVelocity) > 0.1f)
	{
		float swingDirection = (angularVelocity > 0) ? 1.0f : -1.0f;
		float momentumBoost = 0.2f; // 조절 가능
		angularAccel += swingDirection * momentumBoost * (swingInputForce / grappleLength);
	}

	// 각속도 업데이트
	angularVelocity += angularAccel * deltaTime;
	angularVelocity *= swingDamping;  // 감쇠 

	// 충돌 예상 
	float nextAngle = currentAngle + angularVelocity * deltaTime;

	// 새 위치 계산 (원형 운동)
	Vector newPos;
	newPos.x = grapplePoint.x + grappleLength * cosf(currentAngle);
	newPos.y = grapplePoint.y + grappleLength * sinf(currentAngle);

	bool willCollide = false;

	if (bOverlapLeftWall || bOverlapRightWall || bOnGround || bOverlapCeiling)
	{
		willCollide = true;
		angularVelocity = 0; // 충돌 방향으로의 회전 중지
	}

	if (!willCollide) // 충돌이 없으면
	{
		// 위치 업데이트
		currentAngle += angularVelocity * deltaTime;

		Vector newPos;
		newPos.x = grapplePoint.x + grappleLength * cosf(currentAngle);
		newPos.y = grapplePoint.y + grappleLength * sinf(currentAngle);

		owner->SetPosition(newPos);
	}

	// 속도 계산 (해제 시 필요)
	float tangentialSpeed = angularVelocity * grappleLength;

	Vector tangent(-sinf(currentAngle), cosf(currentAngle));
	Vector velocity = tangent * tangentialSpeed;

	owner->SetVelocity(velocity);
}

void PhysicsComponent::UpdateExtendChainPhysics(float deltaTime)
{
	if (!owner) return;

	Player* player = dynamic_cast<Player*>(owner);
	if (!player) return;

	player->GetArmRenderComponent()->SetPosition(player->GetPosition() + Vector(0, -20));

	Vector currentPos = owner->GetPosition();
	Vector toGrapplePoint = grapplePoint - currentPos;
	float distanceToGrapple = toGrapplePoint.Length();

	CollisionComponent* collisionComp = owner->GetComponent<CollisionComponent>();
	if (!collisionComp) return;

	RECT boundingBox = collisionComp->GetBoundingBox();
	float halfWidth = (boundingBox.right - boundingBox.left) / 2.0f;
	float halfHeight = (boundingBox.bottom - boundingBox.top) / 2.0f;

	toGrapplePoint.Normalize();
	float effectiveRadius = max(halfWidth, halfHeight);  // 더 합리적인 계산
	float minDistance = effectiveRadius + 10.0f;  // 여유 거리 추가

	if (distanceToGrapple <= minDistance)
	{
		owner->SetVelocity(Vector(0, 0));

		// 정확한 위치에 배치
		Vector finalPos = grapplePoint - toGrapplePoint * minDistance;
		owner->SetPosition(finalPos);
		return;
	}

	Vector extendVelocity = toGrapplePoint * chainExtendSpeed;
	Vector desiredPos = currentPos + extendVelocity * deltaTime;

	float movedDistance = (desiredPos - currentPos).Length();
	float intendedDistance = (desiredPos - currentPos).Length();

	if (movedDistance > intendedDistance * 0.1f)
	{
		owner->SetVelocity(extendVelocity);
		owner->SetPosition(desiredPos);
	}
	else
	{
		// 거의 못 움직였으면 정지
		owner->SetVelocity(Vector(0, 0));

		// 벽에 딱 붙이기
		if (movedDistance > 0.001f)
			owner->SetPosition(desiredPos);
	}
}


void PhysicsComponent::UpdateCeilingPhysics(float deltaTime)
{
	if (!owner) return;

	Player* player = dynamic_cast<Player*>(owner);
	if (!player) return;

	if(owner->GetDirection().x >= 0)
		player->GetArmRenderComponent()->SetPosition(owner->GetPosition() + Vector(-20, -50));
	else
		player->GetArmRenderComponent()->SetPosition(owner->GetPosition() + Vector(20, -50));

	Vector currentPos = owner->GetPosition();
	Vector direction = owner->GetDirection();

	if (direction.x != 0)
	{
		Vector velocity = owner->GetVelocity();

		if (velocity.x == 0)
			return;

		velocity.x = owner->GetSpeed() * direction.x;
		velocity.y = 0;
		owner->SetVelocity(velocity);

		// 위치 업데이트
		Vector currentPos = owner->GetPosition();
		Vector newPos = currentPos + velocity * deltaTime;
		owner->SetPosition(newPos);
	}

	if (!bOverlapCeiling)
	{
		Vector velocity = owner->GetVelocity();
		if (direction.x != 0)
		{
			Jump();
		}
		else
		{
			velocity.x = 0;
			velocity.y = 0; // 가만히 있었으면 그냥 떨어짐
		}
		SetPhysicsState(EPhysicsState::Normal);
		owner->SetVelocity(velocity);
		return;
	}
	
}

void PhysicsComponent::UpdateClimbingPhysics(float deltaTime)
{
	if (!owner) return;

	Player* player = dynamic_cast<Player*>(owner);
	if (!player) return;

	player->GetArmRenderComponent()->SetPosition(player->GetPosition() + Vector(0, -20));

	Vector direction = owner->GetDirection();
	Vector velocity = owner->GetVelocity();

	velocity.x = 0;

	if (direction.y == 0)
		velocity.y = 0; 

	owner->SetVelocity(velocity);

	Vector newPos = owner->GetPosition() + velocity * deltaTime;
	owner->SetPosition(newPos);
}

void PhysicsComponent::UpdateDash(float deltaTime)
{
	if (!bIsDash || !owner) return;

	dashCurrentTime += deltaTime;

	if (dashCurrentTime >= dashTotalTime)
	{
		owner->SetPosition(dashTargetPostion);
		bIsDash = false;
		dashCurrentTime = 0.0f;

		AirDash();

		SetPhysicsState(EPhysicsState::Normal);

		Player* player = dynamic_cast<Player*>(owner);
		if (!player) return;

		player->EndDash();

		return;
	}

	float progress = dashCurrentTime / dashTotalTime;
	float curvedProgress = GetDashCurve(progress);

	Vector currentPos = owner->GetPosition();
	Vector toTarget = dashTargetPostion - dashStartPostion;
	Vector desiredPos = dashStartPostion + (toTarget * curvedProgress);

	Vector movementDelta = desiredPos - currentPos;
	Vector newPos = currentPos + movementDelta;

	owner->SetPosition(newPos);
}

void PhysicsComponent::Idle()
{
	if (!owner) return;

	// 그래플링 중일 때는 속도를 건드리지 않음 (진자 운동 유지)
	if (physicsState == EPhysicsState::Grappling)
		return;

	Vector velocity = owner->GetVelocity();
	velocity.x = 0;

	if (physicsState == EPhysicsState::RightWallClimbing ||
		physicsState == EPhysicsState::LeftWallClimbing)
		velocity.y = 0;

	owner->SetVelocity(velocity);
}

void PhysicsComponent::Move()
{
	if (!owner) return;

	if (physicsState == EPhysicsState::Grappling)
	{
		swingInputForce += 50.f;
		swingInputForce = clamp(swingInputForce, 0.f, 250.f);
		return;
	}
	else if (physicsState == EPhysicsState::RightWallClimbing ||
		physicsState == EPhysicsState::LeftWallClimbing)
	{
		Vector direction = owner->GetDirection();
		Vector velocity = owner->GetVelocity();

		velocity.x = 0; // 수평 이동 차단

		if (abs(direction.y) > 0.1f) // 부동소수점 오차 고려
			velocity.y = owner->GetSpeed() * direction.y;
		else
			velocity.y = 0; // direction.y가 0이면 정지

		owner->SetVelocity(velocity);
		return;
	}

	Vector direction = owner->GetDirection();
	Vector velocity = owner->GetVelocity();

	if (direction.x > 0 && !bBlockedRight)
		velocity.x = owner->GetSpeed() * direction.x;
	else if (direction.x < 0 && !bBlockedLeft)
		velocity.x = owner->GetSpeed() * direction.x;
	else
		velocity.x = 0;

	owner->SetVelocity(velocity);
}

void PhysicsComponent::Jump()
{
	if (!owner) return;

	if (CanJump() || physicsState == EPhysicsState::CeilingHang)
	{
		Vector velocity = owner->GetVelocity();
		velocity.y = -jumpForce;
		owner->SetVelocity(velocity);

		bOnGround = false;
		bJumping = true;
		bFalling = false;
	}
}

void PhysicsComponent::ReadyForDash()
{
	bRolling = true;
}

void PhysicsComponent::ExtendChain()
{
	if (!owner)
		return;

	SetPhysicsState(EPhysicsState::ExtendingChain);

	Vector currentPos = owner->GetPosition();
	Vector toGrapplePoint = grapplePoint - currentPos;
	float distanceToGrapple = toGrapplePoint.Length();

	// 체인 확장 모드 시작
	bExtendingChain = true;

	// 그래플 포인트 방향으로 직선 이동 시작
	toGrapplePoint.Normalize();
	Vector extendVelocity = toGrapplePoint * chainExtendSpeed;

	owner->SetVelocity(extendVelocity);

	// 각속도는 0으로 설정 (스윙 멈춤)
	angularVelocity = 0.0f;
}

void PhysicsComponent::DashToPosition(Vector position)
{
	if (bIsDash || !owner) return; 

	SetPhysicsState(EPhysicsState::Dashing);

	dashStartPostion = owner->GetPosition();
	dashTargetPostion = position;
	dashCurrentTime = 0.0f;
	bIsDash = true;
}

void PhysicsComponent::EndDash()
{
	bRolling = false;
}

void PhysicsComponent::AirDash()
{
	if (!owner) return;

	Vector direction = owner->GetDirection();

	if (direction.x == 0 && direction.y == 0)
		direction.x = 1.0f;

	float dashForce = 500.0f;       // 수평 속도
	float upwardForce = 300.0f;     // 수직 속도

	Vector velocity = owner->GetVelocity();

	velocity.x = dashForce * direction.x;
	velocity.y = -upwardForce;  // 위쪽으로 (음수)

	owner->SetVelocity(velocity);
}

void PhysicsComponent::KnockBack(Vector collisionNormal)
{
	if (!owner) return;

	Vector knockbackDirection = Vector(collisionNormal.x, -collisionNormal.y);

	float knockbackForce = 200.0f;     
	float upwardForce = 150.0f;       

	Vector velocity = owner->GetVelocity();

	Vector knockbackVelocity;
	knockbackVelocity.x = knockbackForce * knockbackDirection.x;
	knockbackVelocity.y = (collisionNormal.y > 0) ? upwardForce : -upwardForce;

	float blendRatio = 0.7f;  // 70% 넉백
	Vector finalVelocity = velocity * (1.0f - blendRatio) + knockbackVelocity * blendRatio;

	// 최대 속도 제한
	float maxKnockbackSpeed = 200.0f;
	if (finalVelocity.Length() > maxKnockbackSpeed)
	{
		finalVelocity.Normalize();
		finalVelocity *= maxKnockbackSpeed;
	}

	owner->SetVelocity(finalVelocity);
}

void PhysicsComponent::OnGroundBeginOverlap(CollisionComponent* other, HitResult info)
{
	if (other && (other->GetCollisionChannel() == ECollisionChannel::WorldStatic || other->GetCollisionChannel() == ECollisionChannel::WorldDynamic))
	{
		Vector normal = info.collisionNormal;
		Player* player = dynamic_cast<Player*>(owner);
		if (!player) return;

		if (normal.x == 0 && normal.y == -1) // 지면
		{
			SetPhysicsState(EPhysicsState::Normal);
			bOnGround = true;
			bFalling = false;

			player->UpdateMovementState(EPlayerMovementState::Idle);
			player->UpdateActionState(EPlayerActionState::None);
		}
		else if (normal.x == 0 && normal.y == 1) // 천장
		{
			player->UpdateActionState(EPlayerActionState::Ceiling);
			player->UpdateMovementState(EPlayerMovementState::Idle);
			SetPhysicsState(EPhysicsState::CeilingHang);
			bOverlapCeiling = true;
		}
		else if (normal.x == -1 && normal.y == 0) // 오른쪽 벽
		{
			if (bOnGround) 
			{
				bBlockedRight = true;  // 오른쪽 이동 차단 플래그
				return;
			}

			// 공중에서만 벽 붙기
			player->SetDirection(Vector(1, player->GetDirection().x));

			player->UpdateActionState(EPlayerActionState::WallGrab);
			player->UpdateMovementState(EPlayerMovementState::Idle);
			SetPhysicsState(EPhysicsState::RightWallClimbing);
			bOverlapRightWall = true;
		}
		else if (normal.x == 1 && normal.y == 0) // 왼쪽 벽
		{
			if (bOnGround) 
			{
				bBlockedLeft = true;  // 왼쪽 이동 차단 플래그
				return;
			}

			// 공중에서만 벽 붙기
			player->SetDirection(Vector(-1, player->GetDirection().x));

			player->UpdateActionState(EPlayerActionState::WallGrab);
			player->UpdateMovementState(EPlayerMovementState::Idle);
			SetPhysicsState(EPhysicsState::LeftWallClimbing);
			bOverlapLeftWall = true;
		}
	}
}

void PhysicsComponent::OnGroundEndOverlap(CollisionComponent* other, HitResult info)
{
	if (other && (other->GetCollisionChannel() == ECollisionChannel::WorldStatic || other->GetCollisionChannel() == ECollisionChannel::WorldDynamic))
	{
		Vector normal = info.collisionNormal;
		Player* player = dynamic_cast<Player*>(owner);
		if (!player) return;

		if (normal.x == 0 && normal.y == -1) // 지면
		{
			bOnGround = false;

			if (player->GetActionState() == EPlayerActionState::Jump) return;

			player->UpdateActionState(EPlayerActionState::None);
			player->UpdateMovementState(EPlayerMovementState::Fall);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == 0 && normal.y == 1) // 천장
		{
			bOverlapCeiling = false;

			player->UpdateActionState(EPlayerActionState::None);
			player->UpdateMovementState(EPlayerMovementState::Fall);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == -1 && normal.y == 0) // 왼쪽 벽
		{
			bBlockedRight = false;
			bOverlapRightWall = false;

			player->UpdateActionState(EPlayerActionState::None);
			player->UpdateMovementState(EPlayerMovementState::Fall);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == 1 && normal.y == 0) // 오른쪽 벽
		{
			bBlockedLeft = false;
			bOverlapLeftWall = false;

			player->UpdateActionState(EPlayerActionState::None);
			player->UpdateMovementState(EPlayerMovementState::Fall);
			SetPhysicsState(EPhysicsState::Normal);
		}
	}
}

void PhysicsComponent::StartGrappling(Vector projectilePosition)
{
	grapplePoint = projectilePosition;
	grappleLength = (owner->GetPosition() - projectilePosition).Length();
	physicsState = EPhysicsState::Grappling;

	bJustReleasedGrapple = false;
	grappleReleaseTimer = 0.0f;

	Vector velocity = owner->GetVelocity();

	Vector fromGrappleToPlayer = owner->GetPosition() - projectilePosition;
	currentAngle = atan2f(fromGrappleToPlayer.y, fromGrappleToPlayer.x);

	Vector tangent(-sin(currentAngle), cos(currentAngle));
	float tangentialSpeed = velocity.Dot(tangent);
	float velocityRetention = 0.8f;  // 80% 유지

	// 방사 방향과 접선 방향 분리
	Vector radialDir = fromGrappleToPlayer.Normalized();
	float radialSpeed = velocity.Dot(radialDir);
	Vector tangentialVelocity = velocity - radialDir * radialSpeed;

	// 접선 속도만 유지하고 방사 속도는 제거
	owner->SetVelocity(tangentialVelocity * velocityRetention);

	// 각속도 계산
	angularVelocity = tangentialVelocity.Length() / grappleLength;
	if (tangentialVelocity.Dot(tangent) < 0)
		angularVelocity *= -1;

	owner->SetVelocity(tangent * tangentialSpeed);
}

void PhysicsComponent::EndGrappling()
{
	if (physicsState == EPhysicsState::CeilingHang ||
		physicsState == EPhysicsState::RightWallClimbing ||
		physicsState == EPhysicsState::LeftWallClimbing)
		return;

	SetPhysicsState(EPhysicsState::Normal);
	
	bJustReleasedGrapple = true;
	grappleReleaseTimer = 0.0f;

	Player* player = dynamic_cast<Player*>(owner);
	if (player && !bOnGround && player->GetActionState() != EPlayerActionState::TakeDamage)
	{
		player->UpdateMovementState(EPlayerMovementState::Fall);
		player->UpdateActionState(EPlayerActionState::None);
	}

	angularVelocity = 0.0f;
	grappleLength = 0.0f;
}

void PhysicsComponent::SetPhysicsState(EPhysicsState newState)
{
 	 physicsState = newState; 
}

float PhysicsComponent::GetDashCurve(float t)
{
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;

	// 여러 커브 옵션 중 선택

	// 옵션 1: 시작 느림 → 중간 빠름 → 끝 느림 (S-curve)
	// return t * t * (3.0f - 2.0f * t); // smoothstep

	// 옵션 2: 느린 시작 → 급가속 (제곱 커브)
	 return t * t;

	// 옵션 3: 빠른 시작 → 감속 (제곱근 커브)
	// return sqrt(t);

	// 옵션 4: 커스텀 커브 - 0.2초까지는 0.3배속, 이후 0.8배속
	// 
	//float timeThreshold = 0.2f / dashTotalTime; // 전체 시간 대비 비율
	//if (t <= timeThreshold)
	//{
	//	return 0.3f * (t / timeThreshold);
	//}
	//else
	//{
	//	float remainingT = (t - timeThreshold) / (1.0f - timeThreshold);
	//	return 0.3f + 0.8f * remainingT;
	//}
}

