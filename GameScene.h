#pragma once

#include "Scene.h"

class TileMap;
class TileCollisionAdapter;
class Actor;
class Player;
class PlayerController;
class Turret;

class GameScene : public Scene
{
public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	const unordered_map<Actor*, int> GetAllActors() { return actors; }

private:
	unordered_map<Actor*, int> actors; 

	TileMap* tileMap;
	TileCollisionAdapter* tileCollisionApdater = nullptr;

	Player* player = nullptr;
	PlayerController* PC = nullptr;

	Turret* turret = nullptr;

	bool bIsDebug = false;
};

