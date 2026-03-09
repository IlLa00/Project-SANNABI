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
    _buildingMap.clear();
    _enemySpawnMap.clear();
    _platformSpawnMap.clear();

    wchar_t comma;
    file >> _gridWidth >> comma >> _gridHeight >> comma
        >> _tileMapWidth >> comma >> _tileMapHeight;

    _buildingMap.assign(_gridHeight, vector<wstring>(_gridWidth, L""));
    _enemySpawnMap.assign(_gridHeight, vector<bool>(_gridWidth, false));
    _platformSpawnMap.assign(_gridHeight, vector<bool>(_gridWidth, false));

    _layers.resize(3);
    for (auto& layer : _layers)
        layer.tiles.assign(_gridWidth * _gridHeight, -1);

    file.ignore();

    wstring line;
    int currentSection = 0; 

    while (getline(file, line))
    {
        if (line.empty()) continue;

        if (line == L"[Collision]")
        {
            currentSection = 1;
            continue;
        }
        else if (line == L"[Buildings]")
        {
            currentSection = 2;
            continue;
        }
        else if (line == L"[Enemies]")
        {
            currentSection = 3;
            continue;
        }
        else if (line == L"[Platforms]")
        {
            currentSection = 4;
            continue;
        }

        if (currentSection == 0) // 타일 레이어 데이터 처리
        {
            wistringstream iss(line);
            int layerIndex, tileCount;
            wchar_t colon;

            if (iss >> layerIndex >> colon >> tileCount)
            {
                if (layerIndex >= 0 && layerIndex < _layers.size())
                {
                    for (int i = 0; i < tileCount; i++)
                    {
                        getline(file, line);
                        if (!line.empty())
                        {
                            wistringstream tileStream(line);
                            int x, y, encodedValue;

                            if (tileStream >> x >> comma >> y >> comma >> encodedValue)
                            {
                                if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
                                {
                                    // 인코딩된 값을 그대로 저장
                                    _layers[layerIndex].tiles[y * _gridWidth + x] = encodedValue;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (currentSection == 1) // 충돌 영역 데이터 처리 (타입 포함)
        {
            wistringstream iss(line);
            int count = 0;
            if (iss >> count)
            {
                for (int i = 0; i < count; i++)
                {
                    getline(file, line);
                    wistringstream rectStream(line);
                    RECT rect;
                    int typeValue = 0;

                    // 타입 정보가 있는 형식과 없는 형식 모두 처리
                    if (rectStream >> rect.left >> comma >> rect.top >> comma
                        >> rect.right >> comma >> rect.bottom)
                    {
                        // 타입 정보가 있으면 읽기
                        if (rectStream >> comma >> typeValue)
                        {
                            CollisionType type = static_cast<CollisionType>(typeValue);
                            collisionRects.push_back(CollisionRect(rect, type));
                        }
                        else
                        {
                            // 구버전 파일 - 기본 타입으로 추가
                            collisionRects.push_back(CollisionRect(rect, CollisionType::Normal));
                        }
                    }
                }
            }
        }
        else if (currentSection == 2) // 건축물 데이터 처리
        {
            wistringstream buildingStream(line);
            int x, y;
            wchar_t comma;
            wstring path;

            if (buildingStream >> x >> comma >> y >> comma)
            {
                getline(buildingStream, path);

                if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
                {
                    // 현재 실행 파일 경로를 기준으로 절대 경로를 생성합니다.
                    filesystem::path currentDir = filesystem::current_path();
                    filesystem::path relativePath(path);
                    filesystem::path absolutePath = currentDir / relativePath;

                    _buildingMap[y][x] = absolutePath.wstring();
                }
            }
        }
        else if (currentSection == 3) // 적군 데이터 처리
        {
            wistringstream enemyStream(line);
            int x, y;
            wchar_t comma;

            if (enemyStream >> x >> comma >> y)
            {
                if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
                {
                    _enemySpawnMap[y][x] = true;
                }
            }
        }
        else if (currentSection == 4) // 플랫폼 데이터 처리
        {
            wistringstream enemyStream(line);
            int x, y;
            wchar_t comma;

            if (enemyStream >> x >> comma >> y)
            {
                if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
                {
                    _platformSpawnMap[y][x] = true;
                }
            }
        }
    }

    file.close();

    LoadTilesetImage();
    LoadDeathTilesetImage();
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

void TileMap::LoadDeathTilesetImage()
{
    fs::path fullPath = fs::current_path();
    fullPath /= "Level";
    fullPath /= L"Tileset_Death.bmp";  // Death 타일셋 파일

    HDC hdc = GetDC(NULL);
    _hdcDeathTileset = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    _deathTilesetBitmap = (HBITMAP)LoadImageW(
        nullptr,
        fullPath.c_str(),
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    if (_deathTilesetBitmap)
    {
        SelectObject(_hdcDeathTileset, _deathTilesetBitmap);
    }
    else
    {
        // Death 타일셋이 없으면 Normal 타일셋을 대체로 사용
        _hdcDeathTileset = _hdcTileset;
        _deathTilesetBitmap = _tilesetBitmap;
    }
}

CollisionType TileMap::CheckCollisionAt(int x, int y) const
{
    // 픽셀 좌표를 받아서 해당 위치의 충돌 타입 반환
    for (const auto& collision : collisionRects)
    {
        if (x >= collision.rect.left && x < collision.rect.right &&
            y >= collision.rect.top && y < collision.rect.bottom)
            return collision.type;
    }
    return CollisionType::Normal;  
}

bool TileMap::IsDeathTile(int x, int y) const
{
    if (x < 0 || x >= _gridWidth || y < 0 || y >= _gridHeight)
        return false;

    // 모든 레이어 검사
    for (const auto& layer : _layers)
    {
        int encodedValue = layer.tiles[y * _gridWidth + x];
        if (encodedValue >= 0)
        {
            TileInfo tileInfo = TileInfo::Decode(encodedValue);
            if (tileInfo.tilesetType == 1)  // Death 타일셋
                return true;
        }
    }

    // 충돌 영역도 검사 (픽셀 좌표로 변환)
    int pixelX = x * TileSize + TileSize / 2;
    int pixelY = y * TileSize + TileSize / 2;

    for (const auto& collision : collisionRects)
    {
        if (collision.type == CollisionType::Death &&
            pixelX >= collision.rect.left && pixelX < collision.rect.right &&
            pixelY >= collision.rect.top && pixelY < collision.rect.bottom)
            return true;
    }

    return false;
}

const vector<vector<wstring>>& TileMap::GetBuildingMap() const
{
    return _buildingMap;
}

const vector<vector<bool>>& TileMap::GetEnemySpawnMap() const
{
    return _enemySpawnMap;
}

const vector<vector<bool>>& TileMap::GetPlatformSpawnMap() const
{
    return _platformSpawnMap;
}

void TileMap::Render(HDC hdc, Vector cameraPos) 
{
    if (_hdcTileset == nullptr) return;

    // 화면에 보이는 타일 범위만 계산 (최적화)
    int startX = max(0, cameraPos.x / TileSize);
    int endX = min(_gridWidth, (cameraPos.x + GWinSizeX) / TileSize + 1);
    int startY = max(0, cameraPos.y / TileSize);
    int endY = min(_gridHeight, (cameraPos.y + GWinSizeY) / TileSize + 1);

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
            int encodedValue = layer.tiles[y * _gridWidth + x];

            if (encodedValue < 0) continue;  // 빈 타일은 스킵

            // 인코딩된 값에서 타일 정보 추출
            TileInfo tileInfo = TileInfo::Decode(encodedValue);

            // 타일셋 선택
            HDC sourceDC = nullptr;
            if (tileInfo.tilesetType == 0)
                sourceDC = _hdcTileset;  // Normal 타일셋
            else if (tileInfo.tilesetType == 1)
                sourceDC = _hdcDeathTileset;  // Death 타일셋
            else
                sourceDC = _hdcTileset;  // 기본값

            if (sourceDC == nullptr) continue;

            Vector worldPos = Vector(x * TileSize, y * TileSize);

            // 타일셋에서의 위치 계산
            int tileX = tileInfo.tileIndex % _tileMapWidth;
            int tileY = tileInfo.tileIndex / _tileMapWidth;

            TransparentBlt(
                hdc,
                (int)worldPos.x, (int)worldPos.y,
                TileSize, TileSize,  // 대상 크기
                sourceDC,  // 선택된 타일셋 사용
                tileX * OriginTileSize, tileY * OriginTileSize,
                OriginTileSize, OriginTileSize,  // 원본 크기
                RGB(255, 255, 255)  // 흰색을 투명색으로
            );
        }
    }
}