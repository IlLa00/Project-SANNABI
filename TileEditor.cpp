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

	_transparent = RGB(255, 255, 255);

	LoadTileset(TilesetType::Normal, L"Tileset_Ground.bmp");
	LoadTileset(TilesetType::Death, L"Tileset_Death.bmp");  // Death 타일셋 파일명
}

void TileEditor::Update()
{
	if (InputManager::GetInstance()->GetButtonUp(KeyType::Q))
	{
		if (_currentTileset == TilesetType::Normal)
			_currentTileset = TilesetType::Death;
		else
			_currentTileset = TilesetType::Normal;
	}

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

	const wchar_t* tilesetName = (_currentTileset == TilesetType::Normal) ? L"Normal" : L"Death";
	wstring str = std::format(L"Tileset: {0} | Tile x:{1}, y:{2}", tilesetName, x, y);

	if (_currentTileset == TilesetType::Death)
	{
		HBRUSH redBrush = CreateSolidBrush(RGB(255, 200, 200));
		RECT topRect = { 0, 0, _rect.right, 30 };
		FillRect(_hdcBack, &topRect, redBrush);
		DeleteObject(redBrush);
	}

	::TextOut(_hdcBack, 5, 10, str.c_str(), static_cast<int>(str.size()));

	::BitBlt(_hdc, 0, 0, _rect.right, _rect.bottom, _hdcBack, 0, 0, SRCCOPY);
	::PatBlt(_hdcBack, 0, 0, _rect.right, _rect.bottom, WHITENESS);
}

void TileEditor::DrawTileMap(HDC hdc)
{
	TilesetResource& currentTileset = GetCurrentTileset();

	::TransparentBlt(hdc,
		0,
		30,  // Y 오프셋 (텍스트 공간 확보)
		TileMapWidth * OriginTileSize,
		TileMapHeight * OriginTileSize,
		currentTileset.hdcBitmap,
		0,
		0,
		currentTileset.sizeX,
		currentTileset.sizeY,
		_transparent);

	if (_selectedIndex >= 0)
	{
		int x = _selectedIndex % TileMapWidth;
		int y = _selectedIndex / TileMapWidth;

		COLORREF selectColor = (_currentTileset == TilesetType::Normal) ?
			RGB(0, 255, 0) : RGB(255, 0, 0);

		HPEN hPen = CreatePen(PS_SOLID, 3, selectColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));

		Rectangle(hdc,
			x * OriginTileSize,
			y * OriginTileSize + 30,  
			(x + 1) * OriginTileSize,
			(y + 1) * OriginTileSize + 30);

		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
	}
}

bool TileEditor::LoadTileset(TilesetType type, const wstring& fileName)
{
	fs::path fullPath = fs::current_path();
	fullPath /= "Level";
	fullPath /= fileName;

	int index = static_cast<int>(type);
	TilesetResource& tileset = _tilesets[index];

	tileset.fileName = fileName;
	tileset.hdcBitmap = ::CreateCompatibleDC(_hdc);
	tileset.bitmap = (HBITMAP)::LoadImageW(
		nullptr,
		fullPath.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION
	);

	if (!tileset.bitmap)
		return false;

	HBITMAP prevObj = (HBITMAP)::SelectObject(tileset.hdcBitmap, tileset.bitmap);
	::DeleteObject(prevObj);

	BITMAP bit = {};
	::GetObject(tileset.bitmap, sizeof(BITMAP), &bit);

	tileset.sizeX = bit.bmWidth;
	tileset.sizeY = bit.bmHeight;

	return true;
}

TilesetResource& TileEditor::GetCurrentTileset()
{
	return _tilesets[static_cast<int>(_currentTileset)];
}

