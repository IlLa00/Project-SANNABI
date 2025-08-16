#include "pch.h"
#include "GameScene.h"
#include "TileMap.h"
#include "TileCollisionAdapter.h"
#include "Actor.h"
#include "Player.h"
#include "PlayerController.h"
#include "Turret.h"
#include "CameraManager.h"
#include "InputManager.h"

void GameScene::Init()
{
	tileMap = new TileMap();

	fs::path mapPath = fs::current_path();
	mapPath /= "TileMap";
	mapPath /= L"Test.tilemap";  

	if (fs::exists(mapPath)) 
	{
		tileMap->LoadFromFile(mapPath.wstring());
		//MessageBox(NULL, L"타일맵 로드 완료", L"Success", MB_OK);
	}
	else 
		MessageBox(NULL, L"타일맵 파일을 찾을 수 없습니다", L"Warning", MB_OK);

	tileCollisionApdater = new TileCollisionAdapter();
	tileCollisionApdater->LoadFromTileMap(tileMap->GetCollisionRects());

	player = new Player;
	player->Init();
	actors[player]++;

	PC = new PlayerController;
	PC->Posses(player);
	PC->Init();

	turret = new Turret();
	turret->Init();
}

void GameScene::Update(float deltaTime)
{
	for (auto& actor : actors)
	{
		if(actor.first)
			actor.first->Update(deltaTime);
	}

	PC->Update(deltaTime);
	turret->Update(deltaTime);

	if (InputManager::GetInstance()->GetButtonDown(KeyType::Tab))
		bIsDebug = !bIsDebug;
}

void GameScene::Render(HDC _hdcBack)
{
	if (tileMap) 
		tileMap->Render(_hdcBack, CameraManager::GetInstance()->GetCameraPos());

	for (auto& actor : actors)
	{
		if (actor.first)
			actor.first->Render(_hdcBack);
	}

	turret->Render(_hdcBack);

	if (bIsDebug)
		tileCollisionApdater->Render(_hdcBack);
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
}