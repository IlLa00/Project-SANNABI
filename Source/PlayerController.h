#pragma once

class Player;
class InputManager;

class PlayerController
{
public:
	void Init();
	void Update(float deltaTime);
	void Destroy();
	void Posses(Player* character);

private:
	Player* PossedCharacter = nullptr;
	InputManager* inputInstance = nullptr;
};


