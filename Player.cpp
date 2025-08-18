#include "pch.h"
#include "Player.h"
#include "SpriteRenderComponent.h"
#include "CollisionComponent.h"
#include "GrapplingComponent.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "TextureResource.h"
#include "ResourceManager.h"

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
	bodyRenderComponent->AddAnimation("Swing", "SNB_Swing", 15, 1.f);
	bodyRenderComponent->AddAnimation("Ceiling_Idle", "SNB_Ceiling_Idle", 12, 1.f);
	bodyRenderComponent->AddAnimation("Ceiling_Run", "SNB_Ceiling_Run", 16, 1.f);
	bodyRenderComponent->AddAnimation("WallClimb_Idle", "SNB_WallClimb_Idle", 9, 1.f);
	bodyRenderComponent->AddAnimation("WallClimb_Run", "SNB_WallClimb_Run", 10, 1.f);
	bodyRenderComponent->AddAnimation("ChargeStart", "SNB_ChargeDashStart", 23, 0.15f, false);
	bodyRenderComponent->AddAnimation("Charging", "SNB_ChargeDashLoop", 8, 1.f);
	AddComponent(bodyRenderComponent);

	armRenderComponent = new SpriteRenderComponent;
	armRenderComponent->Init(this);
	armRenderComponent->SetScale(0.5f);
	armRenderComponent->AddAnimation("ArmIdle", "SNBArm_Idle", 8, 1.f);
	armRenderComponent->AddAnimation("ArmRun", "SNBArm_Run", 20, 1.f);
	armRenderComponent->AddAnimation("ArmJump", "SNBArm_Jump", 6, 1.0f, false);
	armRenderComponent->AddAnimation("ArmFall", "SNBArm_Fall", 6, 1.0f, false);
	armRenderComponent->AddAnimation("ArmCeiling_Idle", "SNBArm_Ceiling_Idle", 12, 1.f);
	armRenderComponent->AddAnimation("ArmCeiling_Run", "SNBArm_Ceiling_Run", 16, 1.f);
	//armRenderComponent->AddAnimation("ArmWallClimb_Idle", "SNBArm_WallClimb_Idle", 12, 1.f);
	armRenderComponent->AddAnimation("ArmWallClimb_Run", "SNBArm_WallClimb_Run", 10, 1.f);
	AddComponent(armRenderComponent);

	grapplingComponent = new GrapplingComponent;
	grapplingComponent->Init(this);
	AddComponent(grapplingComponent);

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(50, 75);
	collisionComponent->SetCollisionChannel(ECollisionChannel::Character);
	AddComponent(collisionComponent);

	physicsComponent = new PhysicsComponent;
	physicsComponent->Init(this);
	AddComponent(physicsComponent);

	chainPen = CreatePen(PS_DOT, 2, RGB(255, 255, 0));

	movementState = EPlayerMovementState::Idle;
	actionState = EPlayerActionState::None;
}

void Player::Update(float deltaTime)
{
	Super::Update(deltaTime);

	CameraManager::GetInstance()->Update(position, Vector(10000, 10000));

	UpdateAnimation();
}

void Player::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (showAimingLine) // 일단 임시
	{
		Vector mousePos = InputManager::GetInstance()->GetMousePos();

		// 거리 계산
		Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(position);

		float distance = (mousePos - screenPos).Length();
		const float MAX_GRAPPLE_DISTANCE = 400.0f; // 최대 그래플링 거리

		// 거리에 따른 색상 변경
		COLORREF lineColor;
		if (distance <= MAX_GRAPPLE_DISTANCE)
			lineColor = RGB(0, 255, 0);    // 초록색 (사용 가능)
		else
			lineColor = RGB(255, 0, 0);    // 빨간색 (사용 불가)

		// 펜 설정
		HPEN oldPen = (HPEN)SelectObject(_hdcBack, CreatePen(PS_DOT, 2, lineColor));

		// 사슬 조준선 그리기 (점선)
		MoveToEx(_hdcBack, (int)screenPos.x, (int)screenPos.y, NULL);
		LineTo(_hdcBack, (int)mousePos.x, (int)mousePos.y);

		// 마우스 커서 위치에 작은 원 그리기 (조준점)
		HBRUSH oldBrush = (HBRUSH)SelectObject(_hdcBack, CreateSolidBrush(lineColor));
		Ellipse(_hdcBack,
			(int)mousePos.x - 5, (int)mousePos.y - 5,
			(int)mousePos.x + 5, (int)mousePos.y + 5);

		// 리소스 정리
		DeleteObject(SelectObject(_hdcBack, oldPen));
		DeleteObject(SelectObject(_hdcBack, oldBrush));
	}

	DebugRender(_hdcBack);
}

void Player::Destroy()
{
	Super::Destroy();
}

void Player::NoneInput()
{
	if (!physicsComponent) return;
	if (physicsComponent->GetPhysicsState() == EPhysicsState::Grappling || physicsComponent->IsJustRelaeasedGrapple())
		return;

	direction = Vector(0, 0);
	physicsComponent->Idle();

	if (movementState == EPlayerMovementState::Jump) return;
	if (movementState == EPlayerMovementState::Fall) return;

	UpdateMovementState(EPlayerMovementState::Idle);
}

void Player::OnPressA()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::WallGrab) // 벽타기는 좌우이동 없음
		return;

	direction = Vector(-1, 0);

	physicsComponent->Move();

	if(actionState != EPlayerActionState::Jump)
		UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressD()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::WallGrab)
		return;

	direction = Vector(1, 0);

	physicsComponent->Move();

	if (actionState != EPlayerActionState::Jump)
		UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressW()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::Ceiling) // 천장이동은 상하이동 없음
		return;

	direction = Vector(0, -1);

	physicsComponent->Move();

	if (actionState != EPlayerActionState::WallGrab)
		UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnPressS()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::Ceiling) // 천장이동은 상하이동 없음
		return;

	direction = Vector(0, 1);

	physicsComponent->Move();
	UpdateMovementState(EPlayerMovementState::Run);
}

void Player::OnSpaceBarDown()
{
	if (!physicsComponent) return;

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
	if (!grapplingComponent) return;

	Vector mousePos = InputManager::GetInstance()->GetMousePos();
	Vector worldMousePos = CameraManager::GetInstance()->ConvertWorldPos(mousePos);
	Vector mouse_direction = worldMousePos - position;
	mouse_direction.Normalize();

	UpdateActionState(EPlayerActionState::GrappleFire);
	grapplingComponent->FireGrapple(mouse_direction);

	// 애니메이션
}

void Player::OnGrappling(Vector projectilePosition)
{
	if (!physicsComponent) return;

	physicsComponent->StartGrappling(projectilePosition);

	UpdateActionState(EPlayerActionState::GrappleSwing);
	UpdateMovementState(EPlayerMovementState::Idle);
}

void Player::OffGrappling()
{
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

void Player::Dash(Vector position)
{
	// 애니메이션

	UpdateActionState(EPlayerActionState::DashAttack);

	physicsComponent->SetGrapplePoint(position);
	physicsComponent->ExtendChain();
}

void Player::Attack()
{
	if (!target) return;

	
}

void Player::OnMouseUp()
{
	// 그래플링 종료 인데 조건제대로 따져야할듯
	if (actionState == EPlayerActionState::DashAttack)
	{
		// 여기 들어오니까 부수기..
		Attack();
		UpdateActionState(EPlayerActionState::None);
		UpdateMovementState(EPlayerMovementState::Idle);
	}
	else
	{
		grapplingComponent->OffGrappling();
		OffGrappling();
		physicsComponent->EndGrappling();
	}
}

void Player::OnShiftDown()
{
	// 사슬팔이 부착되어있으면 대쉬?

	//if (actionState == EPlayerActionState::Ceiling ||
	//	actionState == EPlayerActionState::GrappleSwing)
	//	return;

	//// 아니면 구르며 공격
	//UpdateActionState(EPlayerActionState::CharageDashStart);
	//physicsComponent->ReadyForDash();
}

void Player::OnShiftUp()
{
	/*UpdateActionState(EPlayerActionState::None);
	physicsComponent->Idle();*/
}

void Player::UpdateMovementState(EPlayerMovementState state)
{
	if (movementState == state)
		return;

	movementState = state;
}

void Player::UpdateActionState(EPlayerActionState state)
{
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
			armRenderComponent->PlayAnimation("ArmIdle");
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("Run");
			armRenderComponent->PlayAnimation("ArmRun");
			break;
		}
		else if (movementState == EPlayerMovementState::Fall)
		{
			bodyRenderComponent->PlayAnimation("Fall");
			armRenderComponent->PlayAnimation("ArmFall");
			break;
		}
		break;
	case EPlayerActionState::Jump:
		if (movementState == EPlayerMovementState::Jump)
		{
			bodyRenderComponent->PlayAnimation("Jump");
			armRenderComponent->PlayAnimation("ArmJump");
			break;
		}
		else if (movementState == EPlayerMovementState::Fall)  // ✅ 추가
		{
			bodyRenderComponent->PlayAnimation("Fall");
			armRenderComponent->PlayAnimation("ArmFall");
		}
		break;
	case EPlayerActionState::GrappleSwing:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("Swing");
			break;
		}
		break;
	case EPlayerActionState::Ceiling:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("Ceiling_Idle");
			armRenderComponent->PlayAnimation("ArmCeiling_Idle");
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("Ceiling_Run");
			armRenderComponent->PlayAnimation("ArmCeiling_Run");
			break;
		}
		break;
	case EPlayerActionState::WallGrab:
		if (movementState == EPlayerMovementState::Idle)
		{
			bodyRenderComponent->PlayAnimation("WallClimb_Idle");
			armRenderComponent->PlayAnimation("ArmWallClimb_Idle");
			break;
		}
		else if (movementState == EPlayerMovementState::Run)
		{
			bodyRenderComponent->PlayAnimation("WallClimb_Run");
			armRenderComponent->PlayAnimation("ArmWallClimb_Run");
			break;
		}
		break;
	}
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

	wstring output = std::format(L"Velocity({:.2f}, {:.2f})", velocity.x, velocity.y);
	::TextOut(_hdcBack, 25, 40, output.c_str(), static_cast<int32>(output.size()));

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
}
