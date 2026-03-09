#pragma once

enum class TilesetType
{
	Normal = 0,
	Death = 1
};

struct TilesetResource
{
	HDC hdcBitmap;
	HBITMAP bitmap;
	int sizeX;
	int sizeY;
	wstring fileName;
};

class TileEditor
{
public:
	void Init(HWND hwnd);
	void Update();
	void Render();

	void SetSelectedTilesetIndex(int index) { _selectedTilesetIndex = index; }

	int GetSelectedIndex() const { return _selectedIndex; }
	TilesetType GetCurrentTilesetType() const { return _currentTileset; }

private:
	// 그리기 함수
	void DrawTileMap(HDC hdc);
	bool LoadTileset(TilesetType type, const wstring& fileName);

	TilesetResource& GetCurrentTileset();

private:
	RECT	_rect;
	HDC		_hdc = {};
	HDC		_hdcBack = {};
	HBITMAP _bmpBack = {};

	TilesetResource _tilesets[2];  // Normal과 Death 타일셋
	TilesetType _currentTileset = TilesetType::Normal;

	int _selectedIndex = -1;

	int _tileMapWidth;
	int _tileMapHeight;

	int _selectedTilesetIndex = 0; // Currently selected tileset
	COLORREF _transparent = RGB(255, 255, 255);

	bool _isDragging = false;
	POINT _prevMousePos = {};
	int _scrollX = 0;
	int _scrollY = 0;
};

