#pragma once

#include "Singleton.h"

class BuildingEditor;

class Engine : public Singleton<Engine>
{
	friend Singleton<Engine>;

protected:
	Engine() {};

public:
	void Init(HWND hwnd, HWND subWnd, HWND sub2Wnd);
	void Destroy();

	void Update();	
	void Render();	

	HWND GetHwnd() { return _hwnd; }

	void SetBuildingEditor(BuildingEditor* editor) { _editor = editor; }
	BuildingEditor* GetBuildingEditor() { return _editor; }

private:
	HWND	_hwnd;	// 윈도우 핸들
	HWND	_hwndSub;
	HWND	_hwndSub2;
	HDC		_hdc;	// 기본 도화지 정보 (프론트 버퍼)

	// Double Buffering
	RECT	_rect;              // 버퍼의 영역
	HDC		_hdcBack = {};	// 여분의 도화지 준비
	HBITMAP _bmpBack = {};	// Bitmap 에다가 써야한다.

	BuildingEditor* _editor = nullptr;
};

