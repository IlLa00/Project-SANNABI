#include "pch.h"
#include "Engine.h"
#include "TimerManager.h"
#include "SceneManager.h"
#include "CollisionManager.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "VFXManager.h"

void Engine::Init(HWND hwnd, HWND subWnd, HWND sub2Wnd)
{
	_hwnd = hwnd;
	_hwndSub = subWnd;
	_hwndSub2 = sub2Wnd;

	// 기본 도화지 넘겨받기
	_hdc = ::GetDC(hwnd);	

	::GetClientRect(hwnd, &_rect);

	// 기본 hdc와 호환되는 DC를 생성
	_hdcBack = ::CreateCompatibleDC(_hdc);

	// hdc와 호환되는 비트맵 생성
	_bmpBack = ::CreateCompatibleBitmap(_hdc, _rect.right, _rect.bottom);

	// DC와 BMP를 연결
	HBITMAP prev = (HBITMAP)::SelectObject(_hdcBack, _bmpBack); 
	::DeleteObject(prev);

	fs::path directory = fs::current_path();
	AddFontResourceEx((directory / L"Font/NotoSansTC-Regular.otf").c_str(), FR_PRIVATE, 0);

	hFont = CreateFont(
		36, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, fontName
	);

	TimerManager::GetInstance()->Init();
	CollisionManager::GetInstance()->Init(hwnd);
	ResourceManager::GetInstance()->Init();

	SoundManager::GetInstance()->Init(hwnd);
	SoundManager::GetInstance()->LoadSound("GameScene", "D:\\KHJ\\Project-SANNABI\\Sound\\BGM_Chap4_Main_Intro.wav", SoundType::BGM);
	SoundManager::GetInstance()->LoadSound("LobbyScene", "D:\\KHJ\\Project-SANNABI\\Sound\\BGM_Event_MainTheme.wav", SoundType::BGM);

	SoundManager::GetInstance()->LoadSound("LobbyLightOn", "D:\\KHJ\\Project-SANNABI\\Sound\\SFX_TitleLightOn.wav", SoundType::SFX);
	SoundManager::GetInstance()->LoadSound("Click", "D:\\KHJ\\Project-SANNABI\\Sound\\SFX_UI_Click.wav", SoundType::SFX);
	SoundManager::GetInstance()->LoadSound("Hover", "D:\\KHJ\\Project-SANNABI\\Sound\\SFX_UI_Hovering.wav", SoundType::SFX);

	VFXManager::GetInstance()->Init();
	VFXManager::GetInstance()->RegisterVFX("ChargeAim", "VFX_ChargeAim_Appear", 9, 1.f, false);
	VFXManager::GetInstance()->RegisterVFX("ChargeAttack", "VFX_ChargeAttack", 7, 1.f, true);
	VFXManager::GetInstance()->RegisterVFX("Fire", "VFX_ShootMuzzleFire", 4, 0.5f, true);
	

	SceneManager::GetInstance()->Init(_hwnd, _hwndSub, _hwndSub2);
}

void Engine::Update()
{
	SceneManager::GetInstance()->Update(TimerManager::GetInstance()->GetDeltaTime());

	TimerManager::GetInstance()->Update();
	CollisionManager::GetInstance()->Update();
	VFXManager::GetInstance()->Update(TimerManager::GetInstance()->GetDeltaTime());

}

void Engine::Destroy()
{
	TimerManager::GetInstance()->DestroyInstance();
	CollisionManager::GetInstance()->DestroyInstance();
	ResourceManager::GetInstance()->DestroyInstance();
	CameraManager::GetInstance()->DestroyInstance();

	DeleteObject(hFont);
	RemoveFontResourceEx(fontPath, FR_PRIVATE, NULL);

	SceneManager::GetInstance()->DestroyInstance();
}


void Engine::Render()
{
	SceneManager::GetInstance()->Render(_hdcBack);
	VFXManager::GetInstance()->Render(_hdcBack);

	uint32 fps = TimerManager::GetInstance()->GetFps();
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	{
		POINT mousePos = InputManager::GetInstance()->GetMousePos();
		wstring str = std::format(L"Mouse({0}, {1})", mousePos.x, mousePos.y);
		::TextOut(_hdcBack, 300, 10, str.c_str(), static_cast<int32>(str.size()));
	}

	{
		wstring str = format(L"FPS({0}), DT({1})", fps, deltaTime);
		::TextOut(_hdcBack, 5, 10, str.c_str(), static_cast<int32>(str.size()));
	}

	// 비트 블릿 : 고속 복사
	::BitBlt(_hdc, 0, 0, _rect.right, _rect.bottom, _hdcBack, 0, 0, SRCCOPY);

	// 프론트 버퍼에 복사가 끝났으면, 백버퍼는 초기화
	::PatBlt(_hdcBack, 0, 0, _rect.right, _rect.bottom, WHITENESS);
}
