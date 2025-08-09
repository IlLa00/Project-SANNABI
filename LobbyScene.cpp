#include "pch.h"
#include "LobbyScene.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "TimerManager.h"

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
}

void  LobbyScene::Render(HDC _hdcBack)
{
	currentBG->Render(_hdcBack, Vector(0, 0));

    if (totalTime >= 3.f)
    {
        UIBG->Render(_hdcBack, Vector(0, 0));
        logo->Render(_hdcBack, Vector(500, -280));
    }
}

void  LobbyScene::Destroy()
{
	SAFE_DELETE(BG1);
	SAFE_DELETE(BG2);
	SAFE_DELETE(BG3);
}