#include "pch.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "LobbyScene.h"
#include "EditorScene.h"

void SceneManager::Init(HWND hwnd, HWND subWnd)
{
	GameScene* gameScene = new GameScene;
	Scenes.push_back(gameScene);

	LobbyScene* lobbyScene = new LobbyScene;
	Scenes.push_back(lobbyScene);

	EditorScene* editorScene = new EditorScene;
	editorScene->SetMainWin(hwnd);
	editorScene->SetSubWin(subWnd);
	Scenes.push_back(editorScene);
	
	currentScene = gameScene;

	currentScene->Init();
}

void SceneManager::Update(float deltaTime)
{
	if (!currentScene) return;

	currentScene->Update(deltaTime);
}

void SceneManager::Render(HDC _hdcBack)
{
	if (!currentScene) return;

	currentScene->Render(_hdcBack);
}

void SceneManager::Destroy()
{
	for (auto& scene : Scenes)
		scene->Destroy();

}