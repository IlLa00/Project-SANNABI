#include "pch.h"
#include "PlayerController.h"
#include "InputManager.h"
#include "Player.h"

void PlayerController::Init()
{
	inputInstance = InputManager::GetInstance();
}

void PlayerController::Update(float deltaTime)
{
	if (!inputInstance) return;
	if (!PossedCharacter) return;

	bool isMoving = false;

    if (inputInstance->GetButtonPressed(KeyType::A))
    {
        PossedCharacter->OnPressA();
        isMoving = true;
    }
    else if (inputInstance->GetButtonPressed(KeyType::D))
    {
        PossedCharacter->OnPressD();
        isMoving = true;
    }
    else if (inputInstance->GetButtonPressed(KeyType::W))
    {
        PossedCharacter->OnPressW();
        isMoving = true;
    }
    else if (inputInstance->GetButtonPressed(KeyType::S))
    {
        PossedCharacter->OnPressS();
        isMoving = true;
    }

    if (inputInstance->GetButtonDown(KeyType::SpaceBar))
    {
        PossedCharacter->OnSpaceBarDown();
    }
    if (inputInstance->GetButtonDown(KeyType::Shift))
    {
        PossedCharacter->OnShiftDown();
    }
    if (inputInstance->GetButtonDown(KeyType::LeftMouse))
    {
        PossedCharacter->OnMouseDown();
    }
    if (inputInstance->GetButtonUp(KeyType::LeftMouse))
    {
        PossedCharacter->OnMouseUp();
    }

    if (!isMoving)
        PossedCharacter->NoneInput();
}

void PlayerController::Destroy()
{
	PossedCharacter = nullptr;
	inputInstance = nullptr;
}

void PlayerController::Posses(Player* character)
{
	if (character)
		PossedCharacter = character;
}