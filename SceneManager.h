#pragma once

#include "Singleton.h"

class Scene;

class SceneManager : public Singleton<SceneManager>
{
	friend Singleton<SceneManager>;

private:
	SceneManager() {}

public:
	void Init(HWND hwnd, HWND subWnd);
	void Update(float deltaTime);
	void Render(HDC _hdcBack);
	void Destroy();

	Scene* GetCurrentScene() { return currentScene; }

private:
	vector<Scene*> Scenes;
	Scene* currentScene;
};

