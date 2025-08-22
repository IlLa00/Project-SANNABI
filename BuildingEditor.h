#pragma once

class BuildingEditor
{
public:
	void Init(HWND hwnd);
	void Update();
	void Render();

	void SetSelectedBitmapPath(const wstring& path); 

	HBITMAP GetSelectedBitmap();
	const wstring& GetSelectedBitmapPath();

private:
	RECT	_rect;
	HDC		_hdc = {};
	HDC		_hdcBack = {};
	HBITMAP _bmpBack = {};

	HBITMAP _hSelectedBitmap = NULL;
	wstring _selectedBitmapPath;
};

