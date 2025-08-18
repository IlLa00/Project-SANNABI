#include "pch.h"
#include "TileEditor.h"
#include "EditorScene.h"
#include "InputManager.h"

void TileEditor::Init(HWND hwnd)
{
	::GetClientRect(hwnd, &_rect);
	_hdc = ::GetDC(hwnd);

	// 더블 버퍼링을 위한것
	_hdcBack = ::CreateCompatibleDC(_hdc); // hdc와 호환되는 DC를 생성
	_bmpBack = ::CreateCompatibleBitmap(_hdc, _rect.right, _rect.bottom); // hdc와 호환되는 비트맵 생성
	HBITMAP prev = (HBITMAP)::SelectObject(_hdcBack, _bmpBack); // DC와 BMP를 연결
	::DeleteObject(prev);

	fs::path fullPath = fs::current_path();
	fullPath /= "Level";
	fullPath /= L"Tileset_Ground.bmp"; // 또는 원하는 단일 파일명

	_hdcBitmap = ::CreateCompatibleDC(_hdc);
	_bitmap = (HBITMAP)::LoadImageW(
		nullptr,
		fullPath.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION
	);

	HBITMAP prevObj = (HBITMAP)::SelectObject(_hdcBitmap, _bitmap);
	::DeleteObject(prevObj);

	BITMAP bit = {};
	::GetObject(_bitmap, sizeof(BITMAP), &bit);

	_sizeX = bit.bmWidth;
	_sizeY = bit.bmHeight;
	_transparent = RGB(255, 255, 255);
}

void TileEditor::Update()
{
	if (InputManager::GetInstance()->GetButtonPressed(KeyType::LeftMouse))
	{
		POINT mousePos = InputManager::GetInstance()->GetSubMousePos();
		// 클릭한 그리드 위치 계산
		int x = mousePos.x / OriginTileSize;
		int y = mousePos.y / OriginTileSize;

		if (x >= 0 && x < TileMapWidth && y >= 0 && y < TileMapHeight)
		{
			_selectedIndex = y * TileMapWidth + x;
		}
	}
}

void TileEditor::Render()
{
	DrawTileMap(_hdcBack); // 백 버퍼에 그리기

	int x = _selectedIndex % TileMapWidth;
	int y = _selectedIndex / TileMapWidth;

	std::wstring str = std::format(L"x:{0}, y:{1}", x, y);
	::TextOut(_hdcBack, 5, 10, str.c_str(), static_cast<int>(str.size()));

	::BitBlt(_hdc, 0, 0, _rect.right, _rect.bottom, _hdcBack, 0, 0, SRCCOPY); // 비트 블릿 : 고속 복사
	::PatBlt(_hdcBack, 0, 0, _rect.right, _rect.bottom, WHITENESS);
}

void TileEditor::DrawTileMap(HDC hdc)
{
	::TransparentBlt(hdc,
		0,
		0,
		TileMapWidth * OriginTileSize,
		TileMapHeight * OriginTileSize,
		_hdcBitmap,
		0,
		0,
		_sizeX,
		_sizeY,
		_transparent);

	if (_selectedIndex >= 0)
	{
		int x = _selectedIndex % TileMapWidth;
		int y = _selectedIndex / TileMapWidth;

		HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, x * OriginTileSize, y * OriginTileSize, (x + 1) * OriginTileSize, (y + 1) * OriginTileSize);
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
	}
}

