#include "pch.h"
#include "GameScene.h"
#include "TileMap.h"
#include "TileCollisionAdapter.h"
#include "Actor.h"
#include "Player.h"
#include "Mari.h"
#include "PlayerController.h"
#include "Turret.h"
#include "Platform.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "TextureResource.h"
#include "SoundManager.h"

#include <locale>
#include <codecvt>

void GameScene::Init()
{
	tileMap = new TileMap();

	fs::path mapPath = fs::current_path();
	mapPath /= "TileMap";
	mapPath /= L"TileMap.tilemap";  

	_BG = new TextureResource();
	_BG->Load(L"./Level/Spr_Chaper4_BG_Sky_batch.bmp");

	if (fs::exists(mapPath)) 
		tileMap->LoadFromFile(mapPath.wstring());

	// 최적화 기법
	HDC hdc = GetDC(_hwnd);
	_hdcMapBack = ::CreateCompatibleDC(hdc);
	_MapbmpBack = ::CreateCompatibleBitmap(hdc, mapSizeX, mapSizeY);

	HBITMAP prev = (HBITMAP)::SelectObject(_hdcMapBack, _MapbmpBack);
	::DeleteObject(prev);

	// 타일맵 배경을 핑크색을 채우기
	::PatBlt(_hdcMapBack, 0, 0, mapSizeX, mapSizeY, RGB(255,255,255));

	const auto& buildingMap = tileMap->GetBuildingMap();
	for (const auto& row : buildingMap)
	{
		for (const auto& path : row)
		{
			if (!path.empty() && _buildingTextures.find(path) == _buildingTextures.end())
			{
				TextureResource* texture = new TextureResource();
				texture->Load(path); 
				_buildingTextures[path] = texture;
			}
		}
	}
	for (int y = 0; y < buildingMap.size(); ++y)
	{
		for (int x = 0; x < buildingMap[y].size(); ++x)
		{
			const wstring& path = buildingMap[y][x];
			if (!path.empty())
			{
				TextureResource* texture = _buildingTextures[path];
				if (texture)
				{
					Vector worldPos(x * TileSize, y * TileSize);

					Vector centeredWorldPos;
					centeredWorldPos.x = worldPos.x + texture->GetSizeX() / 2;
					centeredWorldPos.y = worldPos.y + texture->GetSizeY() / 2;

					texture->Render(_hdcMapBack, centeredWorldPos, false);
				}
			}
		}
	}

	if (tileMap)
		tileMap->Render(_hdcMapBack, CameraManager::GetInstance()->GetCameraPos());


	// 몬스터 위치 조정
	const auto& enemySpawnMap = tileMap->GetEnemySpawnMap();
	for (int y = 0; y < enemySpawnMap.size(); ++y)
	{
		for (int x = 0; x < enemySpawnMap[y].size(); ++x)
		{
			if (enemySpawnMap[y][x])
			{
				Turret* turret = new Turret();
				turret->Init();
				turret->SetPosition(Vector(x * TileSize, y * TileSize)); // TILE_SIZE는 에디터에서 사용한 값과 동일해야 함
				actors[turret]++;
				_turrets.push_back(turret);
			}
		}
	}

	const auto& platformSpawnMap = tileMap->GetPlatformSpawnMap();
	for (int y = 0; y < platformSpawnMap.size(); ++y)
	{
		for (int x = 0; x < platformSpawnMap[y].size(); ++x)
		{
			if (platformSpawnMap[y][x])
			{
				Platform* platform = new Platform();
				platform->Init();
				platform->SetPosition(Vector(x * TileSize, y * TileSize)); // TILE_SIZE는 에디터에서 사용한 값과 동일해야 함
				platform->SetLimitPoint((y - 10) * TileSize, y * TileSize);
				actors[platform]++;
				_platforms.push_back(platform);
			}
		}
	}

	tileCollisionApdater = new TileCollisionAdapter();
	tileCollisionApdater->LoadFromTileMap(tileMap->GetCollisionRects());

	player = new Player;
	player->Init();
	actors[player]++;

	mari = new Mari;
	mari->Init();
	actors[mari]++;

	PC = new PlayerController;
	PC->Posses(player);
	PC->Init();

	ShowCursor(false);

	cursorTexture = new TextureResource();
	cursorTexture->Load("Cursor");

	SoundManager::GetInstance()->PlayBGM("GameScene");
}

void GameScene::Update(float deltaTime)
{
	player->Update(deltaTime);
	mari->Update(deltaTime);

	PC->Update(deltaTime);
	
	for (Turret* turret : _turrets)
		turret->Update(deltaTime);

	for (Platform* platform : _platforms)
		platform->Update(deltaTime);

	if (InputManager::GetInstance()->GetButtonDown(KeyType::Tab))
		bIsDebug = !bIsDebug;
}

void GameScene::Render(HDC _hdcBack)
{
	if (_BG)
	{
		Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(Vector(0, 0));

		// BitBlt를 사용하여 배경을 렌더링합니다.
		::StretchBlt(_hdcBack,
			screenPos.x, screenPos.y,
			mapSizeX, // 대상 너비
			mapSizeY, // 대상 높이
			_BG->_textureHdc,
			0, 0,
			1920,711,
			SRCCOPY); // 원본을 그대로 복사
	}
	
	// 큰 타일맵을 카메라 좌표에 맞게 그린다.
	{
		Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(Vector(0, 0));
		::TransparentBlt(
			_hdcBack,
			screenPos.x, screenPos.y,
			mapSizeX,
			mapSizeY,
			_hdcMapBack,
			0,
			0,
			mapSizeX,
			mapSizeY,
			RGB(255, 255, 255)
		);
	}


	for (Turret* turret : _turrets)
		turret->Render(_hdcBack);

	for (Platform* platform : _platforms)
		platform->Render(_hdcBack);

	player->Render(_hdcBack);
	mari->Render(_hdcBack);

	if (bIsDebug)
		tileCollisionApdater->Render(_hdcBack);

	Vector mousepos = InputManager::GetInstance()->GetMousePos();
	Vector worldMousepos = CameraManager::GetInstance()->ConvertWorldPos(mousepos);
	cursorTexture->Render(_hdcBack, worldMousepos);
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

	for (auto& pair : _buildingTextures)
	{
		SAFE_DELETE(pair.second);
	}
	_buildingTextures.clear();

	for (Turret* turret : _turrets)
		SAFE_DELETE(turret);

	_turrets.clear();

	for (Platform* platform : _platforms)
		SAFE_DELETE(platform);

	_platforms.clear();

	SAFE_DELETE(tileMap);
	SAFE_DELETE(tileCollisionApdater);
	SAFE_DELETE(cursorTexture);
	SAFE_DELETE(_BG);

	SAFE_DELETE(_hdcMapBack);
	SAFE_DELETE(_MapbmpBack);
}