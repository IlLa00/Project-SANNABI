#pragma once

class TileEditor
{
public:
	void Init(HWND hwnd);
	void Update();
	void Render();

	void SetSelectedTilesetIndex(int index) { _selectedTilesetIndex = index; }

	int GetSelectedIndex() const { return _selectedIndex; }
private:
	// 그리기 함수
	void DrawTileMap(HDC hdc);

private:
	RECT	_rect;
	HDC		_hdc = {};
	HDC		_hdcBack = {};
	HBITMAP _bmpBack = {};

	HDC _hdcBitmap;
	HBITMAP _bitmap;
	int _sizeX;
	int _sizeY;
	int _tileMapWidth;
	int _tileMapHeight;

	int _selectedIndex = -1;
	int _selectedTilesetIndex = 0; // Currently selected tileset
	COLORREF _transparent = RGB(255, 255, 255);

	bool _isDragging = false;
	POINT _prevMousePos = {};
	int _scrollX = 0;
	int _scrollY = 0;
};

