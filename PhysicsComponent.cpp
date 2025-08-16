#include "pch.h"
#include "PhysicsComponent.h"
#include "TimerManager.h"
#include "Actor.h"
#include "Player.h"
#include "CollisionComponent.h"
#include "GrapplingComponent.h"

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

					OnGroundEndOverlap(other, info);
				};
		}
	}
}

void PhysicsComponent::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!owner) return;

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
	case EPhysicsState::LeftWallClimbing:
		UpdateClimbingPhysics(deltaTime);
		break;
	}

	Vector velocity = owner->GetVelocity();
	bool velocityChanged = false;

	if (bOnGround)
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

	if (bJumping && velocity.y >= 0)
	{
		bJumping = false;
		bFalling = true; 

		Player* player = dynamic_cast<Player*>(owner);
		if (player)
			player->UpdateMovementState(EPlayerMovementState::Fall);
	}
	else if (!bJumping && !bOnGround && velocity.y > 0)
	{
		if (!bFalling)
		{
			bFalling = true;
			Player* player = dynamic_cast<Player*>(owner);
			if (player)
				player->UpdateMovementState(EPlayerMovementState::Fall);
		}
	}
	else if (bOnGround && bFalling)
		bFalling = false;

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

	currentAngle = atan2f(toHookCurrent.x, toHookCurrent.y);

	// 각가속도 계산 (진자 운동의 기본 공식)
	float angularAccel = -(gravity / grappleLength) * sinf(currentAngle) * swingGravityMult;

	swingInputForce -= 50.0f;
	swingInputForce = clamp(swingInputForce, 0.f, 250.f);

	Vector direction = owner->GetDirection();
	if (direction.x != 0)
	{
		angularAccel += direction.x * (swingInputForce / grappleLength);

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
	newPos.x = grapplePoint.x + grappleLength * sinf(currentAngle);
	newPos.y = grapplePoint.y + grappleLength * cosf(currentAngle);

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
		newPos.x = grapplePoint.x + grappleLength * sinf(currentAngle);
		newPos.y = grapplePoint.y + grappleLength * cosf(currentAngle);

		owner->SetPosition(newPos);
	}

	// 속도 계산 (해제 시 필요)
	float tangentialSpeed = angularVelocity * grappleLength;
	Vector tangent(-cosf(currentAngle), sinf(currentAngle));
	Vector velocity = tangent * tangentialSpeed;

	owner->SetVelocity(velocity);
}

void PhysicsComponent::UpdateExtendChainPhysics(float deltaTime)
{
	if (!owner) return;

	Vector currentPos = owner->GetPosition();
	Vector toGrapplePoint = grapplePoint - currentPos;
	float distanceToGrapple = toGrapplePoint.Length();

	CollisionComponent* collisionComp = owner->GetComponent<CollisionComponent>();
	if (!collisionComp) return;

	RECT boundingBox = collisionComp->GetBoundingBox();

	float halfWidth = (boundingBox.right - boundingBox.left) / 2.0f;
	float halfHeight = (boundingBox.bottom - boundingBox.top) / 2.0f;

	toGrapplePoint.Normalize();

	// 진행 방향에 따른 실제 충돌 크기 계산
	float xExtent = abs(toGrapplePoint.x) * halfWidth;
	float yExtent = abs(toGrapplePoint.y) * halfHeight;
	float colliderRadius = xExtent + yExtent;
	float minDistance = colliderRadius + minGrappleDistance;

	if (distanceToGrapple <= minDistance)
		return;

	toGrapplePoint.Normalize();
	Vector extendVelocity = toGrapplePoint * chainExtendSpeed;
	Vector nextPos = currentPos + extendVelocity * deltaTime;

	owner->SetVelocity(extendVelocity);
	owner->SetPosition(nextPos);
}

void PhysicsComponent::UpdateCeilingPhysics(float deltaTime)
{
	if (!owner) return;

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

	Vector currentPos = owner->GetPosition();
	Vector direction = owner->GetDirection();

	Vector velocity = owner->GetVelocity();

	velocity.x = 0;
	velocity.y = owner->GetSpeed() * direction.y;
	owner->SetVelocity(velocity);

	// 위치 업데이트
	Vector newPos = owner->GetPosition() + velocity * deltaTime;
	owner->SetPosition(newPos);
}

void PhysicsComponent::Idle()
{
	if (!owner) return;

	
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

		velocity.y = owner->GetSpeed() * direction.y;

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
	// 잠깐 포지션 고정
	// 
}

void PhysicsComponent::ExtendChain()
{
	if (physicsState != EPhysicsState::Grappling || !owner)
		return;

	SetPhysicsState(EPhysicsState::ExtendingChain);

	Vector currentPos = owner->GetPosition();
	Vector toGrapplePoint = grapplePoint - currentPos;
	float distanceToGrapple = toGrapplePoint.Length();

	// 이미 충분히 가까이 있으면 실행 안함
	if (distanceToGrapple <= minGrappleDistance)
		return;

	// 체인 확장 모드 시작
	bExtendingChain = true;

	// 그래플 포인트 방향으로 직선 이동 시작
	toGrapplePoint.Normalize();
	Vector extendVelocity = toGrapplePoint * chainExtendSpeed;

	owner->SetVelocity(extendVelocity);

	// 각속도는 0으로 설정 (스윙 멈춤)
	angularVelocity = 0.0f;
}

void PhysicsComponent::OnGroundBeginOverlap(CollisionComponent* other, HitResult info)
{
	if (other && other->GetCollisionChannel() == ECollisionChannel::WorldStatic)
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
			player->UpdateActionState(EPlayerActionState::WallGrab);
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
			player->UpdateActionState(EPlayerActionState::WallGrab);
			SetPhysicsState(EPhysicsState::LeftWallClimbing);
			bOverlapLeftWall = true;
		}
	}
}

void PhysicsComponent::OnGroundEndOverlap(CollisionComponent* other, HitResult info)
{
	if (other && other->GetCollisionChannel() == ECollisionChannel::WorldStatic)
	{
		Vector normal = info.collisionNormal;
		Player* player = dynamic_cast<Player*>(owner);
		if (!player) return;

		if (normal.x == 0 && normal.y == -1) // 지면
		{
			bOnGround = false;

			if (player->GetActionState() == EPlayerActionState::Jump) return;

			player->UpdateActionState(EPlayerActionState::None);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == 0 && normal.y == 1) // 천장
		{
			bOverlapCeiling = false;

			player->UpdateActionState(EPlayerActionState::None);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == -1 && normal.y == 0) // 왼쪽 벽
		{
			bBlockedRight = false;
			bOverlapRightWall = false;

			player->UpdateActionState(EPlayerActionState::None);
			SetPhysicsState(EPhysicsState::Normal);
		}
		else if (normal.x == 1 && normal.y == 0) // 오른쪽 벽
		{
			bBlockedLeft = false;
			bOverlapLeftWall = false;

			player->UpdateActionState(EPlayerActionState::None);
			SetPhysicsState(EPhysicsState::Normal);
		}
	}
}

void PhysicsComponent::StartGrappling(Vector projectilePosition)
{
	grapplePoint = projectilePosition;
	grappleLength = (owner->GetPosition() - projectilePosition).Length();
	physicsState = EPhysicsState::Grappling;

	Vector velocity = owner->GetVelocity();
	toHook = projectilePosition - owner->GetPosition();
	toHook.Normalize();

	// 초기 각도를 여기서 계산하여 저장
	currentAngle = atan2f(toHook.x, toHook.y);

	float radialSpeed = velocity.Dot(toHook);
	Vector tangentialVelocity = velocity - toHook * radialSpeed;
	float tangentialSpeed = tangentialVelocity.Length();

	Vector tangent(-toHook.y, toHook.x);

	if (tangentialVelocity.Dot(tangent) < 0)
		tangentialSpeed *= -1;

	angularVelocity = tangentialSpeed / grappleLength;
}

void PhysicsComponent::EndGrappling()
{
	if (physicsState == EPhysicsState::CeilingHang)
		return;

	if (physicsState == EPhysicsState::RightWallClimbing)
		return;

	if (physicsState == EPhysicsState::LeftWallClimbing)
		return;

	physicsState = EPhysicsState::Normal;

	// 각속도를 선속도로 변환하여 자연스러운 날아가기
	Vector currentPos = owner->GetPosition();
	Vector toHookEnd = currentPos - grapplePoint;
	float angle = atan2f(toHookEnd.x, toHookEnd.y);

	Vector tangent(-cosf(angle), sinf(angle));
	Vector releaseVelocity = tangent * (angularVelocity * grappleLength);

	owner->SetVelocity(releaseVelocity);

	// 변수 초기화
	angularVelocity = 0.0f;
	currentAngle = 0.0f;
}

void PhysicsComponent::SetPhysicsState(EPhysicsState newState)
{
 	 physicsState = newState; 
}
