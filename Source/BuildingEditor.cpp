#include "pch.h"
#include "BuildingEditor.h"
#include "EditorScene.h"
#include "InputManager.h"
#include "Engine.h"

void BuildingEditor::Init(HWND hwnd)
{
	::GetClientRect(hwnd, &_rect);
	_hdc = ::GetDC(hwnd);

	_hdcBack = ::CreateCompatibleDC(_hdc); // hdc와 호환되는 DC를 생성
	_bmpBack = ::CreateCompatibleBitmap(_hdc, _rect.right, _rect.bottom); // hdc와 호환되는 비트맵 생성
	HBITMAP prev = (HBITMAP)::SelectObject(_hdcBack, _bmpBack); // DC와 BMP를 연결
	::DeleteObject(prev);

	Engine::GetInstance()->SetBuildingEditor(this);
}

void BuildingEditor::Update()
{

}

void BuildingEditor::Render()
{
	::BitBlt(_hdc, 0, 0, _rect.right, _rect.bottom, _hdcBack, 0, 0, SRCCOPY);
	::PatBlt(_hdcBack, 0, 0, _rect.right, _rect.bottom, WHITENESS);

	if (_hSelectedBitmap)
	{
		HDC hdcMem = ::CreateCompatibleDC(_hdcBack);
		HBITMAP oldBitmap = (HBITMAP)::SelectObject(hdcMem, _hSelectedBitmap);

		BITMAP bmpInfo;
		::GetObject(_hSelectedBitmap, sizeof(BITMAP), &bmpInfo);

		::TransparentBlt(_hdcBack, 10, 10, bmpInfo.bmWidth, bmpInfo.bmHeight, hdcMem, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, RGB(255, 0, 255));

		::SelectObject(hdcMem, oldBitmap);
		::DeleteDC(hdcMem);
	}
}

void BuildingEditor::SetSelectedBitmapPath(const wstring& path)
{
	if (_hSelectedBitmap)
	{
		::DeleteObject(_hSelectedBitmap);
		_hSelectedBitmap = NULL;
	}

	_selectedBitmapPath = path;

	if (!path.empty())
	{
		_hSelectedBitmap = (HBITMAP)::LoadImageW(
			NULL,
			path.c_str(),
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE
		);
	}
}

HBITMAP BuildingEditor::GetSelectedBitmap()
{
	return _hSelectedBitmap;
}

const wstring& BuildingEditor::GetSelectedBitmapPath()
{
	return _selectedBitmapPath;
}
