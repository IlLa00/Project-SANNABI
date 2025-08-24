#include "pch.h"
#include "LobbyScene.h"
#include "Button.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "TimerManager.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "InputManager.h"

void LobbyScene::Init()
{
	BG1 = new TextureResource();
	BG1->Load("Spr_MainTitle_BlueLED_");

	BG2 = new TextureResource();
	BG2->Load("Spr_MainTitle_LightOn_");

	BG3 = new TextureResource();
	BG3->Load("Spr_MainTitle_LightOff_");

    UIBG = new TextureResource();
    UIBG->Load("UI_MainScene_BG_");
    
    logo = new TextureResource();
    logo->Load("Spr_Logo_Kor_");

	currentBG = BG1;

    playButton = new Button();
    playButton->Init(Vector(1675,700), L"게임 시작");

    editorButton = new Button();
    editorButton->Init(Vector(1675, 800), L"에디터 열기");
        ;
    exitButton = new Button();
    exitButton->Init(Vector(1675, 900), L"게임 종료");

    SoundManager::GetInstance()->PlayBGM("LobbyScene");

    ShowCursor(false);

    cursorTexture = new TextureResource();
    cursorTexture->Load("mouse cursor");
}

void LobbyScene::Update(float deltaTime)
{
    totalTime += deltaTime;

    if (currentBG != BG3)
    {
        if (totalTime >= toBG3Time)
            currentBG = BG3;
        else if (totalTime >= toBG2Time)
            currentBG = BG2;
    }

    if (currentBG == BG3)
    {
        if (fmod(totalTime - toBG3Time, changeCycle) >= changeCycle - changeTime)
            currentBG = BG2;
        else
            currentBG = BG3;
    }

    if (totalTime >= 3.f)
    {
        if (!playButton->GetActive())
            playButton->SetActive(true);

        if (!editorButton->GetActive())
            editorButton->SetActive(true);

        if (!exitButton->GetActive())
            exitButton->SetActive(true);
    }

    playButton->Update();
    editorButton->Update();
    exitButton->Update();

    if (playButton->IsClicked())
    {
        SoundManager::GetInstance()->PlaySound("Click");
        SceneManager::GetInstance()->ChangeScene("GameScene");
    }
    else if (editorButton->IsClicked())
    {
        SoundManager::GetInstance()->PlaySound("Click");
        SceneManager::GetInstance()->ChangeScene("EditorScene");
    }
    else if (exitButton->IsClicked())
    {
        SoundManager::GetInstance()->PlaySound("Click");
        ::PostQuitMessage(0);
    }

   
}

void LobbyScene::Render(HDC _hdcBack)
{
	currentBG->Render(_hdcBack, Vector(0, 0));

    if (totalTime >= 3.f)
    {
        UIBG->Render(_hdcBack, Vector(0, 0));
        logo->Render(_hdcBack, Vector(500, -280));
        
        playButton->Render(_hdcBack);
        editorButton->Render(_hdcBack);
        exitButton->Render(_hdcBack);

        cursorTexture->Render(_hdcBack, InputManager::GetInstance()->GetMousePos());
    }
}

void  LobbyScene::Destroy()
{
	SAFE_DELETE(BG1);
	SAFE_DELETE(BG2);
	SAFE_DELETE(BG3);

    SAFE_DELETE(playButton);
    SAFE_DELETE(editorButton);
    SAFE_DELETE(exitButton);
}