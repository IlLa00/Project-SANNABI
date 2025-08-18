#pragma once

class TileMap
{
public:
    void LoadFromFile(const wstring& filepath);
    void Render(HDC hdc, Vector cameraPos);
    void RenderLayer(HDC hdc, int layerIndex, int startX, int endX, int startY, int endY, Vector cameraPos);
    void LoadTilesetImage();

    Vector GetPlayerSpawnPoint() { return playerSpawnPoint; }
    vector<RECT> GetCollisionRects() { return collisionRects; }
private:
    struct TileLayer
    {
        vector<int> tiles;
    };

    vector<TileLayer> _layers;
    int _gridWidth, _gridHeight;
    int _tileMapWidth, _tileMapHeight;

    HDC _hdcTileset;
    HBITMAP _tilesetBitmap;

    vector<RECT>collisionRects;

    Vector playerSpawnPoint = Vector(10, 100);
};

