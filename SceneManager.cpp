#include "pch.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "LobbyScene.h"
#include "EditorScene.h"

void SceneManager::Init(HWND hwnd, HWND subWnd, HWND subWnd2)
{
	gameScene = new GameScene;
	gameScene->SetHDC(hwnd);
	Scenes.insert({ "GameScene",gameScene });

	lobbyScene = new LobbyScene;
	Scenes.insert({ "LobbyScene",lobbyScene });

	editorScene = new EditorScene;
	editorScene->SetMainWin(hwnd);
	editorScene->SetSubWin(subWnd);
	editorScene->SetSub2Win(subWnd2);
	Scenes.insert({ "EditorScene",editorScene });
	
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
		scene.second->Destroy();

	Scenes.clear();
}

void SceneManager::ChangeScene(string sceneName)
{
	auto it = Scenes.find(sceneName);

	if (it != Scenes.end())
	{
		if (currentScene)
			currentScene->Destroy();

		currentScene = it->second;
		currentScene->Init();
	}
		
}
