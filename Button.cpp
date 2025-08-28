#include "pch.h"
#include "Button.h"
#include "TextureResource.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "CameraManager.h"

void Button::Init(Vector position, const wchar_t* text)
{
	normalTexture = new TextureResource();
	normalTexture->Load("UI_MainScene_SelectBox_batch");
	hoverTexture = new TextureResource();
	hoverTexture->Load("UI_MainScene_SelectBox_Selected_batch");

	_position = position;

	currentTexture = normalTexture;

	buttonRect = { (LONG)_position.x, 
		(LONG)_position.y, 
		(LONG)(_position.x + normalTexture->GetSizeX()),
		(LONG)(_position.y + normalTexture->GetSizeY()) };

	bActive = false;

	buttonText = text;
}

void Button::Update()
{
	if (!bActive) return;

	if (IsMouseOver())
	{
		SoundManager::GetInstance()->PlaySound("Hover");
		currentTexture = hoverTexture;
	}
	else
		currentTexture = normalTexture;
}

void Button::Render(HDC hdc)
{
	if (!bActive) return;

	if (currentTexture)
		currentTexture->Render(hdc, _position, false);

	HFONT oldFont = (HFONT)SelectObject(hdc, hFont); 

	SetBkMode(hdc, TRANSPARENT); // 텍스트 배경 투명
	SetTextColor(hdc, RGB(0, 255, 255)); // 시안색
	TextOut(hdc, _position.x, _position.y, buttonText, wcslen(buttonText));

	SelectObject(hdc, oldFont);
}

bool Button::IsMouseOver()
{
	if (!bActive) return false;

	Vector mousePos = InputManager::GetInstance()->GetMousePos();

	return mousePos.x >= buttonRect.left && mousePos.x <= buttonRect.right &&
		mousePos.y >= buttonRect.top && mousePos.y <= buttonRect.bottom;
}

bool Button::IsClicked()
{
	if (!bActive) return false;

	return IsMouseOver() && InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse);
}
