#pragma once

#include "Scene.h"

class Actor;
class Player;
class PlayerController;

class TestGround;


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

	Player* player = nullptr;
	PlayerController* PC = nullptr;
	TestGround* TG = nullptr;
	TestGround* TG2 = nullptr;
	TestGround* TG3 = nullptr;
};

