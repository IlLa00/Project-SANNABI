#include "pch.h"
#include "TestGround.h"
#include "CollisionComponent.h"

void TestGround::Init()
{
	Super::Init();

	// position = Vector(GWinSizeX / 2, GWinSizeY - 200);

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(800, 125);
	collisionComponent->SetCollisionChannel(ECollisionChannel::WorldStatic);
	AddComponent(collisionComponent);
}

void TestGround::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);
}