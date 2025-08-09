#include "pch.h"
#include "GameScene.h"
#include "Actor.h"
#include "Player.h"
#include "PlayerController.h"


#include "TestGround.h"

void GameScene::Init()
{
	// 여기서 타일맵 읽고? 로드

	player = new Player;
	player->Init();
	actors[player]++;

	PC = new PlayerController;
	PC->Posses(player);
	PC->Init();
	
	TG = new TestGround;
	TG->Init();
	TG->SetPosition(Vector(GWinSizeX / 2, GWinSizeY - 200));
	actors[TG]++;

	TG2 = new TestGround;
	TG2->Init();
	TG2->SetPosition(Vector(GWinSizeX / 2, GWinSizeY /2 - 400));
	actors[TG2]++;

	TG3 = new TestGround;
	TG3->Init();
	TG3->SetPosition(Vector(GWinSizeX - 300, GWinSizeY / 2));
	actors[TG3]++;
}

void GameScene::Update(float deltaTime)
{
	for (auto& actor : actors)
	{
		if(actor.first)
			actor.first->Update(deltaTime);
	}

	PC->Update(deltaTime);
}

void GameScene::Render(HDC _hdcBack)
{
	for (auto& actor : actors)
	{
		if (actor.first)
			actor.first->Render(_hdcBack);
	}
}

void GameScene::Destroy()
{
	for (auto& actor : actors)
	{
		if (actor.first)
			actor.first->Destroy();
	}

	PC->Destroy();
	SAFE_DELETE(PC);

	TG->Destroy();
	SAFE_DELETE(TG);
}