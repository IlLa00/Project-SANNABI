#include "pch.h"
#include "TileMap.h"
#include "CameraManager.h"

void TileMap::LoadFromFile(const wstring& filepath) 
{
    wifstream file(filepath);
    if (!file.is_open()) 
    {
        MessageBox(NULL, L"타일맵 파일을 열 수 없습니다", L"Error", MB_OK);
        return;
    }

    collisionRects.clear();

    wchar_t comma;
    file >> _gridWidth >> comma >> _gridHeight >> comma
        >> _tileMapWidth >> comma >> _tileMapHeight;

    _layers.resize(3); 
    for (auto& layer : _layers) 
        layer.tiles.resize(_gridWidth * _gridHeight, -1);


    wstring line;
    while (std::getline(file, line)) 
    {
        if (line.empty()) continue;

        if (line == L"[Collision]")
        {
            // 충돌 영역 개수 읽기
            getline(file, line);
            int collisionCount = _wtoi(line.c_str());

            // 충돌 영역들 읽기
            for (int i = 0; i < collisionCount; i++)
            {
                if (std::getline(file, line))
                {
                    wistringstream rectStream(line);
                    RECT rect;
                    wchar_t comma;

                    // left,top,right,bottom 형식으로 읽기
                    if (rectStream >> rect.left >> comma >> rect.top >> comma
                        >> rect.right >> comma >> rect.bottom)
                        collisionRects.push_back(rect);
                }
            }
            break;
        }

        wistringstream iss(line);
        int layerIndex, tileCount;
        wchar_t colon;

        if (iss >> layerIndex >> colon >> tileCount) 
        {
            // 타일 데이터 읽기
            for (int i = 0; i < tileCount; i++) 
            {
                std::getline(file, line);
                if (!line.empty()) 
                {
                    wistringstream tileStream(line);
                    int x, y, tileIndex;

                    if (tileStream >> x >> comma >> y >> comma >> tileIndex) 
                    {
                        if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight) 
                            _layers[layerIndex].tiles[y * _gridWidth + x] = tileIndex;
                    }
                }
            }
        }
    }

    file.close();

    // 타일셋 이미지 로드
    LoadTilesetImage();
}

void TileMap::LoadTilesetImage() 
{
    fs::path fullPath = fs::current_path();
    fullPath /= "Level";
    fullPath /= L"Tileset_Ground.bmp";

    HDC hdc = GetDC(NULL);
    _hdcTileset = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    _tilesetBitmap = (HBITMAP)LoadImageW(
        nullptr,
        fullPath.c_str(),
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    SelectObject(_hdcTileset, _tilesetBitmap);
}

void TileMap::Render(HDC hdc, Vector cameraPos) 
{
    if (_hdcTileset == nullptr) return;

    // 화면에 보이는 타일 범위만 계산 (최적화)
    int startX = max(0, cameraPos.x / TileSize);
    int endX = min(_gridWidth, (cameraPos.x + GWinSizeX) / TileSize + 1);
    int startY = max(0, cameraPos.y / TileSize);
    int endY = min(_gridHeight, (cameraPos.y + GWinSizeY) / TileSize + 1);

    // 레이어별로 렌더링 (뒤에서 앞으로)
    for (int layerIndex = 0; layerIndex < _layers.size(); layerIndex++) 
        RenderLayer(hdc, layerIndex, 0, GridWidth, 0, GridHeight, cameraPos);
}

void TileMap::RenderLayer(HDC hdc, int layerIndex, int startX, int endX, int startY, int endY, Vector cameraPos) 
{
    const TileLayer& layer = _layers[layerIndex];

    for (int y = startY; y < endY; y++) 
    {
        for (int x = startX; x < endX; x++) 
        {
            int tileIndex = layer.tiles[y * _gridWidth + x];

            if (tileIndex < 0) continue;  // 빈 타일은 스킵

            // 타일셋에서의 위치 계산
            int tileX = tileIndex % _tileMapWidth;
            int tileY = tileIndex / _tileMapWidth;

            // 화면에 그릴 위치
            Vector screenPos = CameraManager::GetInstance()->ConvertScreenPos(Vector(x * TileSize, y * TileSize));
            int screenX = x * TileSize - cameraPos.x;
            int screenY = y * TileSize - cameraPos.y;

            // 투명색 처리하여 타일 그리기
            TransparentBlt(
                hdc,
                screenPos.x, screenPos.y,
                TileSize, TileSize,  // 대상 크기
                _hdcTileset,
                tileX * OriginTileSize, tileY * OriginTileSize,
                OriginTileSize, OriginTileSize,  // 원본 크기
                RGB(255, 255, 255)  // 흰색을 투명색으로
            );
        }
    }
}