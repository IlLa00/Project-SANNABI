#pragma once

#include "Singleton.h"

class Scene;
class EditorScene;
class GameScene;
class LobbyScene;

class SceneManager : public Singleton<SceneManager>
{
	friend Singleton<SceneManager>;

private:
	SceneManager() {}

public:
	void Init(HWND hwnd, HWND subWnd, HWND subWnd2);
	void Update(float deltaTime);
	void Render(HDC _hdcBack);
	void Destroy();

	Scene* GetCurrentScene() { return currentScene; }
	EditorScene* GetEditorScene() { return editorScene; }

	void ChangeScene(string sceneName);

private:
	unordered_map<string,Scene*> Scenes;
	Scene* currentScene;

	GameScene* gameScene = nullptr;
	LobbyScene* lobbyScene = nullptr;
	EditorScene* editorScene = nullptr;
};

