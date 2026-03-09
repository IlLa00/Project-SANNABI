#pragma once

enum class CollisionType
{
    Normal = 0,    // 일반 벽/지면 충돌
    Death = 1      // 즉사 타일
};

struct CollisionRect
{
    RECT rect;
    CollisionType type;

    CollisionRect() : type(CollisionType::Normal) {}
    CollisionRect(const RECT& r, CollisionType t) : rect(r), type(t) {}
};

// 타일 정보 구조체
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

struct TileLayer
{
    vector<int> tiles;  // 인코딩된 타일 정보
};

class TileMap
{
public:
    void LoadFromFile(const wstring& filepath);
    void Render(HDC hdc, Vector cameraPos);
    void RenderLayer(HDC hdc, int layerIndex, int startX, int endX, int startY, int endY, Vector cameraPos);
    
    void LoadTilesetImage();
    void LoadDeathTilesetImage();
   
    Vector GetPlayerSpawnPoint() { return playerSpawnPoint; }

    const vector<vector<wstring>>& GetBuildingMap() const;
    const vector<vector<bool>>& GetEnemySpawnMap() const;
    const vector<vector<bool>>& GetPlatformSpawnMap() const;
    const vector<CollisionRect>& GetCollisionRects() const { return collisionRects; }

    // 특정 위치의 충돌 체크
    CollisionType CheckCollisionAt(int x, int y) const;
    bool IsDeathTile(int x, int y) const;

private:

    vector<TileLayer> _layers;
    int _gridWidth, _gridHeight;
    int _tileMapWidth, _tileMapHeight;

    HDC _hdcTileset;
    HBITMAP _tilesetBitmap;

    HDC _hdcDeathTileset;
    HBITMAP _deathTilesetBitmap;

    vector<CollisionRect> collisionRects;

    Vector playerSpawnPoint = Vector(10, 100);

    vector<vector<wstring>> _buildingMap;
    vector<vector<bool>> _enemySpawnMap;
    vector<vector<bool>> _platformSpawnMap;
};

