#include "pch.h"
#include "Mari.h"
#include "SpriteRenderComponent.h"
#include "InputManager.h"
#include "CollisionComponent.h"
#include "SceneManager.h"
#include "TextureResource.h"
#include "GameScene.h"
#include "Player.h"
#include "CameraManager.h"

void Mari::Init()
{
	Super::Init();

	position = Vector(3617, 3090);
	direction = Vector(1, 0);

	bodyComponent = new SpriteRenderComponent;
	bodyComponent->Init(this);
	bodyComponent->AddAnimation("Run", "Daughter_Running", 10, 1.f);
	bodyComponent->AddAnimation("Idle", "Daughter_Idle", 12, 1.f);
	AddComponent(bodyComponent);

	collisionComponent = new CollisionComponent;
	collisionComponent->Init(this);
	collisionComponent->SetCollisionSize(25, 30);
	collisionComponent->SetCollisionChannel(ECollisionChannel::Character);
	collisionComponent->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
		{
			OnBeginOverlap(other, info);
		};
	AddComponent(collisionComponent);

	endTextrue = new TextureResource();
	endTextrue->Load("END");
}

void Mari::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (!bEndGame)
	{
		bodyComponent->PlayAnimation("Run");
		position = position + direction * 200.f * deltaTime;
	}
	else
	{
		InputManager::GetInstance()->SetEndGameMode(true);

		bodyComponent->PlayAnimation("Idle");
		endGameTimer += deltaTime;

		if (endGameTimer >= 1.0f && !bShowGameOverScreen)
			bShowGameOverScreen = true;

		if (endGameTimer >= 3.0f)
			::PostQuitMessage(0);
	}
}

void Mari::Render(HDC _hdcBack)
{
	Super::Render(_hdcBack);

	if (bShowGameOverScreen)
	{
		int imageWidth = endTextrue->GetSizeX();
		int imageHeight = endTextrue->GetSizeY();

		int destX = GWinSizeX / 2 - imageWidth / 2;
		int destY = GWinSizeY / 2 - imageHeight / 2;

		::StretchBlt(_hdcBack,
			destX, destY,
			imageWidth, imageHeight,
			endTextrue->_textureHdc,
			0, 0,
			imageWidth, imageHeight,
			SRCCOPY);
	}
}

void Mari::Destroy()
{
	Super::Destroy();
}

void Mari::OnBeginOverlap(CollisionComponent* other, HitResult info)
{
	if (!other) return;

	if (other->GetCollisionChannel() == ECollisionChannel::WorldStatic)
	{
		if (direction.x == 1)
			direction.x = -1;
		else
			direction.x = 1;
	}
	else if (other->GetCollisionChannel() == ECollisionChannel::Character || 
		other->GetCollisionChannel() == ECollisionChannel::CharacterInvincible)
	{
		GameScene* scene = dynamic_cast<GameScene*>(SceneManager::GetInstance()->GetCurrentScene());
		if (!scene) return;

		unordered_map<Actor*, int> allActors = scene->GetAllActors();

		for (const auto& pair : allActors)
		{
			Actor* actor = pair.first;

			Player* player = dynamic_cast<Player*>(actor);
			if (!player) continue;

			if (player == other->GetOwner())
			{
				bEndGame = !bEndGame;
				return;
			}
		}
	}
}