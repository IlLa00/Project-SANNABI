#pragma once

#include "Scene.h"

class TextureResource;
class Button;

class LobbyScene : public Scene
{
public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

private:
	TextureResource* BG1 = nullptr;
	TextureResource* BG2 = nullptr;
	TextureResource* BG3 = nullptr;

	TextureResource* currentBG = nullptr;

	TextureResource* UIBG = nullptr;
	TextureResource* logo = nullptr;
	
	Button* playButton;
	Button* editorButton;
	Button* exitButton;

	float toBG2Time = 3.f;
	float toBG3Time = 3.1f;

	float changeCycle = 5.f;
	float changeTime = 0.15f;

	float totalTime = 0.0f;
};

