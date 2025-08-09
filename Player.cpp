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

	position = Vector(GWinSizeX / 2 - 300, GWinSizeY / 2);

	bodyRenderComponent = new SpriteRenderComponent;
	bodyRenderComponent->Init(this);
	bodyRenderComponent->AddAnimation("Idle", "SNB_Idle", 8, 1.f);
	bodyRenderComponent->AddAnimation("Run", "SNB_Run", 22, 3.f);
	bodyRenderComponent->PlayAnimation("Idle");
	AddComponent(bodyRenderComponent);

	armRenderComponent = new SpriteRenderComponent;
	armRenderComponent->Init(this);
	armRenderComponent->AddAnimation("ArmIdle", "SNBArm_Idle", 8, 1.f);
	armRenderComponent->PlayAnimation("ArmIdle");
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
}

void Player::Update(float deltaTime)
{
	Super::Update(deltaTime);

	CameraManager::GetInstance()->Update(position, Vector(GWinSizeX, GWinSizeY));
}

void Player::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (showAimingLine) // 일단 임시
	{
		Vector mousePos = InputManager::GetInstance()->GetMousePos();

		// 거리 계산
		float distance = (mousePos - position).Length();
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
		MoveToEx(_hdcBack, (int)position.x, (int)position.y, NULL);
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

	direction = Vector(0, 0);

	physicsComponent->Idle(); // 이건 맞음

	//근데 이건 모르겠음
	
	if(actionState != EPlayerActionState::Ceiling && 
		actionState != EPlayerActionState::WallGrab)
		UpdateMovementState(EPlayerMovementState::Idle);

	switch (movementState)
	{
	case EPlayerMovementState::Idle:
		bodyRenderComponent->PlayAnimation("Idle");
		armRenderComponent->PlayAnimation("ArmIdle");
		break;
	}
}

void Player::OnPressA()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::WallGrab) // 벽타기는 좌우이동 없음
		return;

	direction = Vector(-1, 0);

	physicsComponent->Move();

	if (actionState != EPlayerActionState::Ceiling)
		UpdateMovementState(EPlayerMovementState::Run);

	switch (movementState)
	{
	case EPlayerMovementState::Run:
		// 나중에 액션 고려..
		
		bodyRenderComponent->PlayAnimation("Run");
		//armRenderComponent->PlayAnimation("ArmRun");
		break;
	}
}

void Player::OnPressD()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::WallGrab)
		return;

	direction = Vector(1, 0);

	physicsComponent->Move();
	
	if (actionState != EPlayerActionState::Ceiling)
		UpdateMovementState(EPlayerMovementState::Run);

	switch (movementState)
	{
	case EPlayerMovementState::Run:
		// 나중에 액션 고려..
		
		bodyRenderComponent->PlayAnimation("Run");
		//armRenderComponent->PlayAnimation("ArmRun");
		break;
	}
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

	switch (movementState)
	{
	case EPlayerMovementState::Run:
		bodyRenderComponent->PlayAnimation("Run");
		//armRenderComponent->PlayAnimation("ArmRun");
		break;
	}
}

void Player::OnPressS()
{
	if (!physicsComponent) return;
	if (actionState == EPlayerActionState::Ceiling) // 천장이동은 상하이동 없음
		return;

	direction = Vector(0, 1);

	physicsComponent->Move();
	UpdateMovementState(EPlayerMovementState::Run);

	switch (movementState)
	{
	case EPlayerMovementState::Run:
		// 나중에 액션 고려..
		
		bodyRenderComponent->PlayAnimation("Run");
		//armRenderComponent->PlayAnimation("ArmRun");
		break;
	}
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

		UpdateMovementState(EPlayerMovementState::Jump);
		// armRenderComponent->PlayAnimation("ArmJump");
	}
}

void Player::OnMouseDown()
{
	if (!grapplingComponent) return;

	// 기본적으로 사슬팔 발사
	// 안되는 구간이? 일단 천장이랑 벽에 붙어있을때? ㄴㄴ 다됨

	Vector mousePos = InputManager::GetInstance()->GetMousePos();
	Vector mouse_direction = mousePos - position;
	mouse_direction.Normalize();

	UpdateActionState(EPlayerActionState::GrappleFire);
	grapplingComponent->FireGrapple(mouse_direction);

	// 애니메이션
}

void Player::OnGrappling(Vector projectilePosition)
{
	if (!physicsComponent) return;

	physicsComponent->StartGrappling(projectilePosition);

	// UpdateMovementState(EPlayerMovementState::Grappling);
	UpdateActionState(EPlayerActionState::GrappleSwing);
}

void Player::OffGrappling()
{
	UpdateMovementState(EPlayerMovementState::Idle); // 진짜 이건가?
	UpdateActionState(EPlayerActionState::None);
}

void Player::OnMouseUp()
{
	// 그래플링 종료 인데 조건제대로 따져야할듯

	OffGrappling();
	physicsComponent->EndGrappling();
	grapplingComponent->OffGrappling();

}

void Player::OnShiftDown()
{
	// 사슬팔이 부착되어있으면 대쉬
	
	// 아니면 구르며 공격

	/*UpdateActionState(EPlayerActionState::Attack); 
	physicsComponent->ReadyForDash();*/
}

void Player::UpdateMovementState(EPlayerMovementState state)
{
	movementState = state;
}

void Player::UpdateActionState(EPlayerActionState state)
{
	actionState = state;
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
	case EPlayerActionState::Attack:
		return L"Attack";
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
	wstring output = std::format(L"Velocity({:.2f}, {:.2f})", velocity.x, velocity.y);
	::TextOut(_hdcBack, 25, 25, output.c_str(), static_cast<int32>(output.size()));

	wstring movementStr = GetMovementStateString(movementState);
	wstring actionStr = GetActionStateString(actionState);
	wstring physicsStr = GetPhysicsStateString(physicsComponent->GetPhysicsState());
	wstring outputStr = L"Movement(" + movementStr + L"), Action(" + actionStr + L"), Physics(" + physicsStr + L")";
	::TextOut(_hdcBack, 25, 50, outputStr.c_str(), static_cast<int32>(outputStr.size()));

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
