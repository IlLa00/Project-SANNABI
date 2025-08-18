#include "pch.h"
#include "CollisionComponent.h"
#include "Actor.h"
#include "CameraManager.h"

void CollisionComponent::Init(Actor* _owner)
{
	Super::Init(_owner);

	CollisionManager::GetInstance()->RegisterCollisionComponent(this);
}

void CollisionComponent::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (GetOwner())
		position = GetOwner()->GetPosition();
}

void CollisionComponent::Render(HDC hdc)
{
	Super::Render(hdc);

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); 
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(position);

	int left = screenPos.x - offsetX;
	int top = screenPos.y - offsetY;
	int right = screenPos.x + offsetX;
	int bottom = screenPos.y + offsetY;

	MoveToEx(hdc, left, top, NULL);
	LineTo(hdc, right, top);
	LineTo(hdc, right, bottom);
	LineTo(hdc, left, bottom);
	LineTo(hdc, left, top);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

void CollisionComponent::Destroy()
{
	Super::Destroy();
}

void CollisionComponent::SetCollisionSize(int _offsetX, int _offsetY)
{
	offsetX = _offsetX;
	offsetY = _offsetY;

	width = 2 * offsetX;
	height = 2 * offsetY;
}

RECT CollisionComponent::GetBoundingBox() const
{
	RECT rect;
	rect.left = position.x - offsetX;
	rect.top = position.y - offsetY;
	rect.right = position.x + offsetX;
	rect.bottom = position.y + offsetY;
	return rect;
}
