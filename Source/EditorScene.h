#pragma once

#include "Scene.h"

class TileEditor;
class BuildingEditor;

enum class EEditMode
{
	Tile,
	Collision,
	Building,
	Enemy,
	Platform
};

enum class CollisionType
{
	Normal = 0,
	Death = 1
};

struct CollisionRect
{
	RECT rect;
	CollisionType type;

	CollisionRect(const RECT& r, CollisionType t) : rect(r), type(t) {}
};

struct TileInfo
{
	int tileIndex;
	int tilesetType;  // 0=Normal, 1=Death

	static TileInfo Decode(int encodedValue)
	{
		TileInfo info;
		info.tileIndex = encodedValue & 0xFFFF;
		info.tilesetType = (encodedValue >> 16) & 0xFFFF;
		return info;
	}
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
	void UpdateEditBuildingMode();
	void UpdateEditEnemyMode();
	void UpdateEditPlatformMode();

	void SetMainWin(HWND _mainWnd) { mainWnd = _mainWnd; }
	void SetSubWin(HWND _subWnd) { subWnd = _subWnd; }
	void SetSub2Win(HWND _sub2Wnd) { sub2Wnd = _sub2Wnd; }

	void SetEditMode();
	
	int GetSelectedTileIndex();
	void DrawMainGrid(HDC hdc);
	void DrawTileOnGrid(HDC hdc, int layer, int gridX, int gridY);
	void DrawBuildings(HDC hdc); 
	void DrawEnemySpawns(HDC hdc);
	void DrawPlatformSpawns(HDC hdc);

	void DrawCollisionRects(HDC hdc);

	void SaveTileMap();
	void LoadTileMap();

	HBITMAP AddBitmap(const wstring& filePath);

private:
	HWND mainWnd;
	HWND subWnd; 
	HWND sub2Wnd;

	TileEditor* tileEditor = nullptr;
	BuildingEditor* buildingEditor = nullptr;

	HWND	_hwnd;
	RECT	_rect;
	HDC		_hdc = {};
	HDC		_hdcBack = {};
	HBITMAP _bmpBack = {};

	HDC		_hdcBitmap = {};
	HBITMAP _bitmap = 0;

	HDC _hdcDeathBitmap = {};
	HBITMAP _deathBitmap = 0;

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

	bool bOnDeathTile = false;

	EEditMode editmode = EEditMode::Tile;
	POINT dragStartPos;
	POINT dragEndPos;
	vector<CollisionRect> collisionRects;
	CollisionType currentCollisionType = CollisionType::Normal;

	vector<vector<wstring>> _buildingMap;
	map<wstring, HBITMAP> _loadedBitmaps;

	vector<vector<bool>> _enemySpawnMap;

	vector<vector<bool>> _platformSpawnMap;
};

