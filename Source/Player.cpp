#include "pch.h"
#include "Player.h"
#include "CollisionManager.h"
#include "VFXManager.h"
#include "SpriteRenderComponent.h"
#include "CollisionComponent.h"
#include "GrapplingComponent.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "TextureResource.h"
#include "ResourceManager.h"
#include "GrapplingHookProjectile.h"

void Player::Init()
{
	Super::Init();

	position = Vector(GWinSizeX / 2 - 300, GWinSizeY / 2 - 100);

	bodyRenderComponent = new SpriteRenderComponent;
	bodyRenderComponent->Init(this);
	bodyRenderComponent->AddAnimation("Idle", "SNB_Idle", 8, 1.f);
	bodyRenderComponent->AddAnimation("Run", "SNB_Run", 18, 1.0f);
	bodyRenderComponent->AddAnimation("Jump", "SNB_Jump", 6, 1.0f, false);
	bodyRenderComponent->AddAnimation("Fall", "SNB_Fall", 6, 1.0f, false);
	bodyRenderComponent->AddAnimation("Shot", "SNB_Shot", 4, 0.2f, false);
	bodyRenderComponent->AddAnimation("Swing", "SNB_Swing", 14, 1.f);
	bodyRenderComponent->AddAnimation("Ceiling_Idle", "SNB_Ceiling_Idle", 12, 1.f);
	bodyRenderComponent->AddAnimation("Ceiling_Run", "SNB_Ceiling_Run", 16, 1.f);
	bodyRenderComponent->AddAnimation("WallClimb_Idle", "SNB_WallClimb_Idle", 9, 1.f);
	bodyRenderComponent->AddAnimation("WallClimb_Run", "SNB_WallClimb_Run", 10, 1.f);
	bodyRenderComponent->AddAnimation("ChargeStart", "SNB_ChargeDashStart", 23, 1.0f, false);
	bodyRenderComponent->AddAnimation("Charging", "SNB_ChargeDashLoop", 8, 1.f);
	bodyRenderComponent->AddAnimation("Dash", "SNB_Dash", 4, 0.3f);
	bodyRenderComponent->AddAnimation("AirDash", "SNB_ExcDash", 17, 1.f, false);
	bodyRenderComponent->AddAnimation("TakeDamage", "SNB_Damaged", 5, 0.75f);
	AddComponent(bodyRenderComponent);

	armRenderComponent = new SpriteRenderComponent;
	armRenderComponent->Init(this);
	armRenderComponent->SetTransformMode(ETransformMode::Relative);
	armRenderComponent->SetScale(0.5f);
	armRenderComponent->SetRotationPivot({ 0.f,0.5f });

	armRenderComponent->AddAnimation("ArmIdle", "SNBArm_Idle", 8, 1.f);
	armRenderComponent->AddAnimation("ArmRun", "SNBArm_Run", 20, 1.f);
	armRenderComponent->AddAnimation("ArmJump", "SNBArm_Jump", 6, 1.0f, false);
	armRenderComponent->AddAnimation("ArmFall", "SNBArm_Fall", 6, 1.0f, false);
	armRenderComponent->AddAnimation("ArmSwing", "SNBArm_Winding", 10, 1.0f, false);
	armRenderComponent->AddAnimation("ArmCeiling_Idle", "SNBArm_Ceiling_Idle", 12, 1.f);
	armRenderComponent->AddAnimation("ArmCeiling_Run", "SNBArm_Ceiling_Run", 16, 1.f);
	AddComponent(armRenderComponent);

	grapplingComponent = new GrapplingComponent;
	grapplingComponent->Init(this);
	AddComponent(grapplingComponent);

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(50, 60);
	collisionComponent->SetCollisionChannel(ECollisionChannel::Character);
	AddComponent(collisionComponent);

	physicsComponent = new PhysicsComponent;
	physicsComponent->Init(this);
	AddComponent(physicsComponent);

	chainPen = CreatePen(PS_DOT, 2, RGB(255, 255, 0));

	movementState = EPlayerMovementState::Idle;
	actionState = EPlayerActionState::None;

	chainLinkTexture = new TextureResource();
	chainLinkTexture->Load("Chain");
}

void Player::Update(float deltaTime)
{
	Super::Update(deltaTime);

	CameraManager::GetInstance()->Update(position, Vector(mapSizeX, mapSizeY), deltaTime);

	if (actionState == EPlayerActionState::GrappleSwing ||
		actionState == EPlayerActionState::GrappleReelIn)
	{
		armRenderComponent->SetUseRotation(true);

		Vector grapplePoint = physicsComponent->GetCurrentProjectile()->GetPosition();
		Vector direction = grapplePoint - position;
		float armAngle = atan2(direction.y, direction.x);
		armRenderComponent->SetRotation(armAngle);
	}
	else if (actionState == EPlayerActionState::GrappleFire)
	{
		armRenderComponent->SetUseRotation(true);

		Vector mousePoint = InputManager::GetInstance()->GetMousePos();
		Vector direction = mousePoint - position;
		float armAngle = atan2(direction.y, direction.x);
		armRenderComponent->SetRotation(armAngle);

	}
	else
		armRenderComponent->SetUseRotation(false);


	if (bHasDelayedFunction)
	{
		delayTimer1 -= deltaTime;
		delayTimer2 -= deltaTime;

		if (delayTimer1 <= 0.0f)
		{
			if (delayedFunctionPtr1)
				(this->*delayedFunctionPtr1)();

			delayTimer1 = 0.0f;
			delayedFunctionPtr1 = nullptr;
		}
		if (delayTimer2 <= 0.0f)
		{
			if (delayedFunctionPtr2)
				(this->*delayedFunctionPtr2)();

			delayTimer2 = 0.0f;
			delayedFunctionPtr2 = nullptr;
		}

		if (!delayedFunctionPtr1 && !delayedFunctionPtr2)
			bHasDelayedFunction = false;
	}

	if (actionState == EPlayerActionState::CharageDashStart ||
		actionState == EPlayerActionState::ChargeReady)
	{
		chargingTimer += deltaTime;

		if (chargingTimer >= chargeTime && !bHasDetected) // 1초가 지나면
		{
			UpdateActionState(EPlayerActionState::ChargeReady);
			detectComponents = CollisionManager::GetInstance()->DetectEnemiesInRange(position, 1000.f, collisionComponent);

			target = nullptr;
			float minDistSq = FLT_MAX;

			for_each(detectComponents.begin(), detectComponents.end(),
				[this, &minDistSq](CollisionComponent* comp)
				{
					if (comp && comp->GetOwner())
					{
						float distSq = (comp->GetOwner()->GetPosition() - position).LengthSquared();
						if (distSq < minDistSq)
						{
							minDistSq = distSq;
							target = comp->GetOwner();

							VFXManager::GetInstance()->Play("ChargeAim", target->GetPosition());

						}
					}
				});
			bHasDetected = true;
		}

		if (chargingTimer >= maxChargeTime)
			UpdateActionState(EPlayerActionState::None);
	}

	UpdateAnimation();
}

void Player::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (actionState == EPlayerActionState::GrappleSwing ||
		actionState == EPlayerActionState::GrappleFire ||
		actionState == EPlayerActionState::GrappleReelIn)
	{
		Vector startPos = position;

		if (!grapplingComponent->GetCurrentProjectile()) return;

		Vector endPos = grapplingComponent->GetCurrentProjectile()->GetPosition();

		if (!chainLinkTexture) return;

		Vector direction = endPos - startPos;
		float length = direction.Length();

		direction.Normalize();

		float linkSize = chainLinkTexture->GetSizeY();
		int linkCount = (int)ceil(length / linkSize);

		float angle = atan2(direction.y, direction.x) * 180.0f / 3.14159f + 90.0f;

		for (int i = 0; i < linkCount; ++i)
		{
			Vector linkPos = startPos + direction * (i * linkSize);

			chainLinkTexture->Render(_hdcBack, linkPos);
		}
	}

	DebugRender(_hdcBack);
}

void Player::Destroy()
{
	SAFE_DELETE(chainLinkTexture);
	
	detectComponents.clear();

	Super::Destroy();
}

void Player::OnCharacterBeginOverlap(CollisionComponent* other, HitResult info)
{
	if (actionState == EPlayerActionState::TakeDamage) return;
	if (bDamaging) return;

	Super::OnCharacterBeginOverlap(other, info);

	if (other->GetCollisionChannel() == ECollisionChannel::Character &&
		actionState == EPlayerActionState::DashAttack)
	{
		Attack();
		
		return;
	}
	if (actionState == EPlayerActionState::GrappleFire ||
		actionState == EPlayerActionState::GrappleReelIn ||
		actionState == EPlayerActionState::GrappleSwing)
		OffGrappling();
	
	if(other->GetCollisionChannel() != ECollisionChannel::Character)
		physicsComponent->KnockBack(info.collisionNormal);

	UpdateActionState(EPlayerActionState::TakeDamage);

	bDamaging = true; // 얘는 한 0.3초?
	collisionComponent->SetCollisionChannel(ECollisionChannel::CharacterInvincible); // 얘는 한 1초

	SetDelayedFunction(&Player::OnDelayedControlRecovery, &Player::OnDelayedCollisionRecovery, 0.5f, 1.f);

	CameraManager::GetInstance()->StartCameraShake(0.5f, 5.f);
}

void Player::SetDelayedFunction(void(Player::* func1)(), void(Player::* func2)(), float delay1, float delay2)
{
	delayedFunctionPtr1 = func1;
	delayedFunctionPtr2 = func2;
	delayTimer1 = delay1;
	delayTimer2 = delay2;
	bHasDelayedFunction = true;
}

void Player::NoneInput()
{
	if (!physicsComponent) return;
	if (bDamaging) return;
	if (physicsComponent->GetPhysicsState() == EPhysicsState::Grappling || physicsComponent->IsJustRelaeasedGrapple())
		return;

	physicsComponent->Idle();

	if (movementState == EPlayerMovementState::Jump) return;
	if (movementState == EPlayerMovementState::Fall) return;

	UpdateMovementState(EPlayerMovementState::Idle);
}

void Player::OnPressA()
{
	if (!physicsComponent) return;
	if (bDamaging) return;
	if (actionState == EPlayerActionState::WallGrab ||
		actionState == EPlayerActionState::DashAttack) // 벽타기는 좌우이동 없음
		return;

	direction = Vector(-1, 0);
	lastDirection = direction;

	physicsComponent->Move();

	if (actionState != EPlayerActionState::Jump)
		UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressD()
{
	if (!physicsComponent) return;
	if (bDamaging) return;
	if (actionState == EPlayerActionState::WallGrab ||
		actionState == EPlayerActionState::DashAttack)
		return;

	direction = Vector(1, 0);
	lastDirection = direction;

	physicsComponent->Move();

	if (actionState != EPlayerActionState::Jump)
		UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressW()
{
	if (!physicsComponent) return;
	if (bDamaging) return;
	if (actionState == EPlayerActionState::Ceiling) // 천장이동은 상하이동 없음
		return;

	direction.y = -1;
	lastDirection = direction;

	physicsComponent->Move();

	UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressS()
{
	if (!physicsComponent) return;
	if (bDamaging) return;
	if (actionState == EPlayerActionState::Ceiling) // 천장이동은 상하이동 없음
		return;

	direction.y = 1;
	lastDirection = direction;

	physicsComponent->Move();
	UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnSpaceBarDown()
{
	if (!physicsComponent) return;
	if (bDamaging) return;

	// 사슬걸고 있을때는 사슬감기
	// 아니면 점프
	if (actionState == EPlayerActionState::GrappleSwing)
	{
		physicsComponent->ExtendChain();
		UpdateActionState(EPlayerActionState::GrappleReelIn);
	}
	else if (actionState == EPlayerActionState::Ceiling ||
		actionState == EPlayerActionState::WallGrab)
	{
		// 천장에서 떼어내어야함
		physicsComponent->SetPhysicsState(EPhysicsState::Normal);

		UpdateMovementState(EPlayerMovementState::Idle);
		UpdateActionState(EPlayerActionState::None);
	}
	else
	{
		physicsComponent->Jump();

		UpdateActionState(EPlayerActionState::Jump);
		UpdateMovementState(EPlayerMovementState::Jump);
	}
}

void Player::OnMouseDown()
{
	if (bDamaging) return;
	if (!grapplingComponent) return;

	if (actionState == EPlayerActionState::GrappleFire ||
		actionState == EPlayerActionState::GrappleReelIn ||
		actionState == EPlayerActionState::GrappleSwing)
		return;

	Vector mousePos = InputManager::GetInstance()->GetMousePos();
	Vector worldMousePos = CameraManager::GetInstance()->ConvertWorldPos(mousePos);
	Vector mouse_direction = worldMousePos - position;
	mouse_direction.Normalize();

	UpdateActionState(EPlayerActionState::GrappleFire);
	grapplingComponent->FireGrapple(mouse_direction);

	VFXManager::GetInstance()->Play("Fire", position, mouse_direction);
}

void Player::OnGrappling(GrapplingHookProjectile* curprojectile)
{
	if (!physicsComponent) return;

	physicsComponent->StartGrappling(curprojectile);

	UpdateActionState(EPlayerActionState::GrappleSwing);
	UpdateMovementState(EPlayerMovementState::Idle);
}

void Player::OffGrappling()
{
	physicsComponent->EndGrappling();
	grapplingComponent->OffGrappling();

	if (physicsComponent->GetPhysicsState() == EPhysicsState::RightWallClimbing ||
		physicsComponent->GetPhysicsState() == EPhysicsState::LeftWallClimbing)
		return;

	if (physicsComponent->GetPhysicsState() == EPhysicsState::CeilingHang)
		UpdateActionState(EPlayerActionState::Ceiling);
	else if (!physicsComponent->IsOnGround()) // 공중에 있으면 Fall 상태
	{
		UpdateMovementState(EPlayerMovementState::Fall);
		UpdateActionState(EPlayerActionState::None);
	}
	else
	{
		UpdateMovementState(EPlayerMovementState::Idle);
		UpdateActionState(EPlayerActionState::None);
	}
}

void Player::Dash()
{
	if (!target) return;

	UpdateActionState(EPlayerActionState::DashAttack);

	bAttacking = true;

	physicsComponent->DashToPosition(target->GetPosition());
}

void Player::EndDash()
{
	Attack();

	target = nullptr;
	bHasDetected = false;
}

void Player::Attack()
{
	if (!target) return;

	target->TakeDamage();

	bAttacking = false;

	physicsComponent->AirDash();
	UpdateActionState(EPlayerActionState::AirDash);

	target = nullptr;

	if (detectComponents.size() > 0)
		detectComponents.clear();

	if (chargingTimer > 0.f)
		chargingTimer = 0.f;

	CameraManager::GetInstance()->StartCameraShake(0.5f, 5.f);
}

void Player::OnMouseUp()
{
	if (bDamaging) return;

	if (actionState == EPlayerActionState::DashAttack)
		Attack();
	else if (actionState == EPlayerActionState::GrappleSwing
		|| actionState == EPlayerActionState::GrappleReelIn)
		OffGrappling();
}

void Player::OnShiftDown()
{
	if (bDamaging) return;

	if (actionState == EPlayerActionState::Ceiling ||
		actionState == EPlayerActionState::GrappleSwing)
		return;

	UpdateActionState(EPlayerActionState::CharageDashStart);

	physicsComponent->ReadyForDash();

	VFXManager::GetInstance()->Play("ChargeAttack", position);
}

void Player::OnShiftUp()
{
	if (bDamaging) return;

	if (actionState == EPlayerActionState::ChargeReady)
	{
		if (target)
		{
			UpdateActionState(EPlayerActionState::DashAttack); // 나중에 바꾸기 애니메이션 재생이 다름
			physicsComponent->DashToPosition(target->GetPosition());

			VFXManager::GetInstance()->Stop("ChargeAim");
		}
		else
		{
			UpdateActionState(EPlayerActionState::None);
			physicsComponent->EndDash();
		}
	}
	else
	{
		UpdateActionState(EPlayerActionState::None);
		physicsComponent->EndDash();
	}
}

void Player::UpdateMovementState(EPlayerMovementState state)
{
	if (movementState == state)
		return;

	if (movementState == EPlayerMovementState::Fall && state == EPlayerMovementState::Run)
		return;

	movementState = state;
}

void Player::UpdateActionState(EPlayerActionState state)
{
	if (bAttacking || bDamaging) return;

	actionState = state;
}

void Player::UpdateAnimation()
{
	switch (actionState)
	{
	case EPlayerActionState::None:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("Idle");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmIdle");
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("Run");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmRun");
			break;
		}
		else if (movementState == EPlayerMovementState::Fall)
		{
			bodyRenderComponent->PlayAnimation("Fall");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmFall");
			break;
		}
		break;
	case EPlayerActionState::Jump:
		if (movementState == EPlayerMovementState::Jump)
		{
			bodyRenderComponent->PlayAnimation("Jump");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmJump");
			break;
		}
		else if (movementState == EPlayerMovementState::Fall)
		{
			bodyRenderComponent->PlayAnimation("Fall");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmFall");
		}
		break;
	case EPlayerActionState::GrappleFire:
		bodyRenderComponent->PlayAnimation("Shot");
		break;
	case EPlayerActionState::GrappleSwing:
		bodyRenderComponent->PlayAnimation("Swing");

		armRenderComponent->SetVisibility(true);
		armRenderComponent->PlayAnimation("ArmSwing");
		break;
	case EPlayerActionState::Ceiling:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("Ceiling_Idle");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmCeiling_Idle");
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("Ceiling_Run");

			armRenderComponent->SetVisibility(true);
			armRenderComponent->PlayAnimation("ArmCeiling_Run");
			break;
		}
		break;
	case EPlayerActionState::WallGrab:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("WallClimb_Idle");
			armRenderComponent->SetVisibility(false);
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("WallClimb_Run");
			armRenderComponent->SetVisibility(false);
			break;
		}
		break;
	case EPlayerActionState::CharageDashStart:
	{
		bodyRenderComponent->PlayAnimation("ChargeStart");
		armRenderComponent->SetVisibility(false);
		break;
	}
	case EPlayerActionState::ChargeReady:
	{
		bodyRenderComponent->PlayAnimation("Charging");
		armRenderComponent->SetVisibility(false);
		break;
	}
	case EPlayerActionState::AirDash:
	{
		bodyRenderComponent->PlayAnimation("AirDash");
		armRenderComponent->SetVisibility(false);
		break;
	}
	case EPlayerActionState::TakeDamage:
	{
		bodyRenderComponent->PlayAnimation("TakeDamage");
		armRenderComponent->SetVisibility(false);
		break;
	}
	case EPlayerActionState::DashAttack:
		bodyRenderComponent->PlayAnimation("Dash");
		armRenderComponent->SetVisibility(false);
		break;
	}
}

void Player::OnDelayedControlRecovery()
{
	bDamaging = false;

	UpdateActionState(EPlayerActionState::None);
}

void Player::OnDelayedCollisionRecovery()
{
	collisionComponent->SetCollisionChannel(ECollisionChannel::Character);
}

const wchar_t* Player::GetMovementStateString(EPlayerMovementState state)
{
	switch (state)
	{
	case EPlayerMovementState::Idle:
		return L"Idle";
	case EPlayerMovementState::Run:
		return L"Run";
	case EPlayerMovementState::Jump:
		return L"Jump";
	case EPlayerMovementState::Fall:
		return L"Fall";
	default:
		return L"Unknown";
	}
}

const wchar_t* Player::GetActionStateString(EPlayerActionState state)
{
	switch (state)
	{
	case EPlayerActionState::None:
		return L"Idle";
	case EPlayerActionState::Jump:
		return L"Jump";
	case EPlayerActionState::CharageDashStart:
		return L"CharageDashStart";
	case EPlayerActionState::ChargeReady:
		return L"ChargeReady";
	case EPlayerActionState::GrappleFire:
		return L"GrappleFire";
	case EPlayerActionState::GrappleSwing:
		return L"GrappleSwing";
	case EPlayerActionState::GrappleReelIn:
		return L"GrappleReelIn";
	case EPlayerActionState::Ceiling:
		return L"Ceiling";
	case EPlayerActionState::WallGrab:
		return L"WallGrab";
	case EPlayerActionState::TakeDamage:
		return L"TakeDamage";
	case EPlayerActionState::Die:
		return L"Die";
	default:
		return L"Unknown";
	}
}

const wchar_t* Player::GetPhysicsStateString(EPhysicsState state)
{
	switch (state)
	{
	case EPhysicsState::Normal:
		return L"Normal";
	case EPhysicsState::Grappling:
		return L"Grappling";
	case EPhysicsState::CeilingHang:
		return L"CeilingHang";
	case EPhysicsState::ExtendingChain:
		return L"ExtendingChain";
	case EPhysicsState::LeftWallClimbing:
		return L"LeftWallClimbing";
	case EPhysicsState::RightWallClimbing:
		return L"RightWallClimbing";
	default:
		return L"Unknown";
	}
}

void Player::DebugRender(HDC _hdcBack)
{
	wstring positionoutput = std::format(L"Postion({:.2f}, {:.2f})", GetPosition().x, GetPosition().y);
	::TextOut(_hdcBack, 25, 25, positionoutput.c_str(), static_cast<int32>(positionoutput.size()));

	{
		wstring output = std::format(L"Velocity({:.2f}, {:.2f}) / Direction({:.2f}, {:.2f})", velocity.x, velocity.y, direction.x, direction.y);
		::TextOut(_hdcBack, 25, 40, output.c_str(), static_cast<int32>(output.size()));
	}

	wstring movementStr = GetMovementStateString(movementState);
	wstring actionStr = GetActionStateString(actionState);
	wstring physicsStr = GetPhysicsStateString(physicsComponent->GetPhysicsState());
	wstring outputStr = L"Movement(" + movementStr + L"), Action(" + actionStr + L"), Physics(" + physicsStr + L")";
	::TextOut(_hdcBack, 25, 60, outputStr.c_str(), static_cast<int32>(outputStr.size()));

	bool isOnGround = physicsComponent->IsOnGround();
	wstring outputGroundStr = L"IsOnGround(" + wstring(GetBoolString(isOnGround)) + L")";
	::TextOut(_hdcBack, 25, 75, outputGroundStr.c_str(), static_cast<int32>(outputGroundStr.size()));

	bool isOnCeiling = physicsComponent->IsOnCeiling();
	wstring outputCeilingStr = L"IsOnCeiling(" + wstring(GetBoolString(isOnCeiling)) + L")";
	::TextOut(_hdcBack, 25, 100, outputCeilingStr.c_str(), static_cast<int32>(outputCeilingStr.size()));

	bool isOnLeftWall = physicsComponent->IsOnLeftWall();
	wstring outputLeftStr = L"IsOnLeftWall(" + wstring(GetBoolString(isOnLeftWall)) + L")";
	// 3. TextOut은 그대로 사용
	::TextOut(_hdcBack, 25, 125, outputLeftStr.c_str(), static_cast<int32>(outputLeftStr.size()));

	bool isOnRightWall = physicsComponent->IsOnRightWall();
	wstring outputRightWallStr = L"IsOnRightWall(" + wstring(GetBoolString(isOnRightWall)) + L")";
	// 3. TextOut은 그대로 사용
	::TextOut(_hdcBack, 25, 150, outputRightWallStr.c_str(), static_cast<int32>(outputRightWallStr.size()));

	{
		Vector camerapos = CameraManager::GetInstance()->GetCameraPos();
		wstring output = std::format(L"ca.x({:.2f},ca.y({:.2f}", camerapos.x, camerapos.y);
		// 3. TextOut은 그대로 사용
		::TextOut(_hdcBack, 25, 150, output.c_str(), static_cast<int32>(output.size()));
	}
}
