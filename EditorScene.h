#pragma once

#include "Scene.h"

class TileEditor;

enum class EEditMode
{
	Tile,
	Collision
};

class EditorScene : public Scene
{
public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render(HDC _hdcBack) override;
	void Destroy() override;

	void UpdateEditTileMode();
	void UpdateEditCollisionMode();

	void SetMainWin(HWND _mainWnd) { mainWnd = _mainWnd; }
	void SetSubWin(HWND _subWnd) { subWnd = _subWnd; }
	
	void SetEditMode();
	
	int GetSelectedTileIndex();
	void DrawMainGrid(HDC hdc);
	void DrawTileOnGrid(HDC hdc, int layer, int gridX, int gridY);
	
	void DrawCollisionRects(HDC hdc);

	void SaveTileMap();
	void LoadTileMap();

private:
	HWND mainWnd;
	HWND subWnd; 

	TileEditor* tileEditor = nullptr;

	HWND	_hwnd;
	RECT	_rect;
	HDC		_hdc = {};
	HDC		_hdcBack = {};
	HBITMAP _bmpBack = {};

	HDC		_hdcBitmap = {};
	HBITMAP _bitmap = 0;
	int32 _transparent = 0;
	int32 _sizeX = 0;
	int32 _sizeY = 0;

	const static int _layerCount = 4;
	int _selectedLayer = 0;
	struct TileLayer
	{
		std::vector<int> mainGrid; // 메인 창 그리드 데이터
		int GetValidCount();
	};
	TileLayer _layer[_layerCount]; // 레이어 배열 정의

	bool _isDragging = false;      // 드래그 상태 초기화
	bool _isDrawCurrLayer = false;

	Vector cameraPosition;
	float cameraSpeed = 10.f;

	EEditMode editmode = EEditMode::Tile;
	POINT dragStartPos;
	POINT dragEndPos;
	vector<RECT> collisionRects;
};

