#pragma once

#include "Scene.h"

class TileMap;
class TileCollisionAdapter;
class Actor;
class Player;
class Mari;
class PlayerController;
class Turret;
class TextureResource;
class Platform;

class GameScene : public Scene
{
public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void SetHDC(HWND hwnd) { _hwnd = hwnd; }

	const unordered_map<Actor*, int> GetAllActors() { return actors; }

private:
	unordered_map<Actor*, int> actors; 

	TileMap* tileMap;
	TileCollisionAdapter* tileCollisionApdater = nullptr;

	Player* player = nullptr;
	PlayerController* PC = nullptr;

	Turret* turret = nullptr;

	Mari* mari = nullptr;

	bool bIsDebug = false;

	TextureResource* cursorTexture = nullptr;

	map<wstring, TextureResource*> _buildingTextures;
	vector<Turret*> _turrets;

	vector<Platform*> _platforms;

	TextureResource* _BG;

	HWND _hwnd;
	HDC		_hdcMapBack = {};	// 여분의 도화지 준비
	HBITMAP _MapbmpBack = {};	// Bitmap 에다가 써야한다.
};

