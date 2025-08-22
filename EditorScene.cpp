#include "pch.h"
#include "EditorScene.h"
#include "TileEditor.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "BuildingEditor.h"

void EditorScene::Init()
{
	tileEditor = new TileEditor();
	tileEditor->Init(subWnd);

	buildingEditor = new BuildingEditor();
	buildingEditor->Init(sub2Wnd);
	_buildingMap.resize(GridHeight, vector<wstring>(GridWidth, L""));

	_enemySpawnMap.resize(GridHeight, vector<bool>(GridWidth, false));
	_platformSpawnMap.resize(GridHeight, vector<bool>(GridWidth, false));

	// 그리드 초기화
	for (int i = 0; i < _layerCount; i++)
		_layer[i].mainGrid.resize(GridWidth * GridHeight, -1);

	{
		fs::path fullPath = fs::current_path();
		fullPath /= "Level";
		fullPath /= L"Tileset_Ground.bmp";

		_hdcBitmap = ::CreateCompatibleDC(_hdc);
		_bitmap = (HBITMAP)::LoadImageW(
			nullptr,
			fullPath.c_str(),
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION
		);

		HBITMAP prev = (HBITMAP)::SelectObject(_hdcBitmap, _bitmap);
		::DeleteObject(prev);

		BITMAP bit = {};
		::GetObject(_bitmap, sizeof(BITMAP), &bit);

		_sizeX = bit.bmWidth;
		_sizeY = bit.bmHeight;
		_transparent = RGB(255, 255, 255);
	}
	{
		fs::path fullPath = fs::current_path();
		fullPath /= "Level";
		fullPath /= L"Tileset_Death.bmp";  // Death 타일셋 파일명

		_hdcDeathBitmap = ::CreateCompatibleDC(_hdc);
		_deathBitmap = (HBITMAP)::LoadImageW(
			nullptr,
			fullPath.c_str(),
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION
		);

		if (_deathBitmap)
		{
			HBITMAP prev = (HBITMAP)::SelectObject(_hdcDeathBitmap, _deathBitmap);
			::DeleteObject(prev);
		}
	}

	for (int i = 0; i < _layerCount; i++)
		_layer[i].mainGrid.resize(GridWidth * GridHeight, -1);

	ShowWindow(subWnd, true);
	UpdateWindow(subWnd);

	ShowWindow(sub2Wnd, true);
	UpdateWindow(sub2Wnd);
}

void EditorScene::Update(float deltaTime)
{
	HWND hwnd = ::GetForegroundWindow();

	buildingEditor->Update();

	if (hwnd == subWnd && subWnd)
	{
		tileEditor->Update();
		return;
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
		_isDragging = true;

	if (editmode == EEditMode::Tile)
		UpdateEditTileMode();
	else if (editmode == EEditMode::Collision)
		UpdateEditCollisionMode();
	else if (editmode == EEditMode::Building)
		UpdateEditBuildingMode();
	else if (editmode == EEditMode::Enemy)
		UpdateEditEnemyMode();
	else if (editmode == EEditMode::Platform)
		UpdateEditPlatformMode();

	if (InputManager::GetInstance()->GetButtonUp(KeyType::T))
		SaveTileMap();
	if (InputManager::GetInstance()->GetButtonUp(KeyType::L))
		LoadTileMap();
	if (InputManager::GetInstance()->GetButtonUp(KeyType::Tab))
		SetEditMode();

	if (InputManager::GetInstance()->GetButtonUp(KeyType::LeftMouse))
		_isDragging = false;

	if (InputManager::GetInstance()->GetButtonPressed(KeyType::W))
		cameraPosition.y -= cameraSpeed;
	else if (InputManager::GetInstance()->GetButtonPressed(KeyType::S))
		cameraPosition.y += cameraSpeed;
	else if (InputManager::GetInstance()->GetButtonPressed(KeyType::A))
		cameraPosition.x -= cameraSpeed;
	else if (InputManager::GetInstance()->GetButtonPressed(KeyType::D))
		cameraPosition.x += cameraSpeed;
}

void EditorScene::Render(HDC _hdcBack)
{
	buildingEditor->Render();
	DrawBuildings(_hdcBack);

	tileEditor->Render();
	DrawMainGrid(_hdcBack); // 백 버퍼에 그리기

	DrawCollisionRects(_hdcBack);

	DrawEnemySpawns(_hdcBack);

	DrawPlatformSpawns(_hdcBack);

	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	int x = mousePos.x / TileSize;
	int y = mousePos.y / TileSize;

	{
		const wchar_t* modestr;
		if (editmode == EEditMode::Tile)
			modestr = L"Tile";
		else if (editmode == EEditMode::Collision)
		{
			if (bOnDeathTile)
				modestr = L"Collision (Death)";
			else
				modestr = L"Collision (Normal)";
		}
		else if (editmode == EEditMode::Building)
			modestr = L"Building";
		else
			modestr = L"Enemy";

		wstring str = format(L"Layer:{0}, State:{1}", _selectedLayer, modestr);
		::TextOut(_hdcBack, 5, 10, str.c_str(), static_cast<int>(str.size()));
	}

	{
		wstring str = format(L"x:{0}, y:{1}", x, y);
		::TextOut(_hdcBack, 5, 30, str.c_str(), static_cast<int>(str.size()));
	}
}

void EditorScene::Destroy()
{

}

void EditorScene::UpdateEditTileMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	int worldX = (mousePos.x + cameraPosition.x) / TileSize;
	int worldY = (mousePos.y + cameraPosition.y) / TileSize;

	if (_isDragging && InputManager::GetInstance()->GetButtonPressed(KeyType::LeftMouse))
	{
		if (worldX >= 0 && worldX < GridWidth && worldY >= 0 && worldY < GridHeight)
		{
			int selectedTileIndex = tileEditor->GetSelectedIndex();
			if (selectedTileIndex >= 0)
			{
				int tilesetType = (tileEditor->GetCurrentTilesetType() == TilesetType::Death) ? 1 : 0;

				int encodedValue = (tilesetType << 16) | (selectedTileIndex & 0xFFFF);
				_layer[_selectedLayer].mainGrid[worldY * GridWidth + worldX] = encodedValue;
			}
		}
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::RightMouse))
	{
		int worldX = (mousePos.x + cameraPosition.x) / TileSize;
		int worldY = (mousePos.y + cameraPosition.y) / TileSize;

		if (worldX >= 0 && worldX < GridWidth && worldY >= 0 && worldY < GridHeight)
			_layer[_selectedLayer].mainGrid[worldY * GridWidth + worldX] = -1;
	}
}

void EditorScene::UpdateEditCollisionMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	int worldX = mousePos.x + cameraPosition.x;
	int worldY = mousePos.y + cameraPosition.y;

	// 그리드 좌표
	int gridX = worldX / TileSize;
	int gridY = worldY / TileSize;

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
	{
		dragStartPos = { gridX, gridY };
		dragEndPos = { gridX, gridY };
	}
	if (InputManager::GetInstance()->GetButtonPressed(KeyType::LeftMouse))
		dragEndPos = { gridX, gridY };

	if (InputManager::GetInstance()->GetButtonDown(KeyType::Q))
	{
		bOnDeathTile = !bOnDeathTile;
		currentCollisionType = bOnDeathTile ? CollisionType::Death : CollisionType::Normal;
	}

	// 마우스 왼쪽 버튼 뗌 - 충돌 영역 생성
	if (InputManager::GetInstance()->GetButtonUp(KeyType::LeftMouse))
	{
		// 직사각형 생성 (좌상단, 우하단 정렬)
		RECT collisionRect;
		collisionRect.left = min(dragStartPos.x, dragEndPos.x) * TileSize;
		collisionRect.top = min(dragStartPos.y, dragEndPos.y) * TileSize;
		collisionRect.right = (max(dragStartPos.x, dragEndPos.x) + 1) * TileSize;
		collisionRect.bottom = (max(dragStartPos.y, dragEndPos.y) + 1) * TileSize;

		collisionRects.push_back(CollisionRect(collisionRect, currentCollisionType));
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::RightMouse))
	{
		vector<CollisionRect>::iterator it = collisionRects.begin();
		while (it != collisionRects.end())
		{
			if (worldX >= it->rect.left && worldX < it->rect.right &&
				worldY >= it->rect.top && worldY < it->rect.bottom)
				it = collisionRects.erase(it);
			else
				++it;
		}
	}
}

void EditorScene::UpdateEditBuildingMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();

	int worldX = mousePos.x + cameraPosition.x;
	int worldY = mousePos.y + cameraPosition.y;

	int gridX = worldX / TileSize;
	int gridY = worldY / TileSize;

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
	{
		const wstring& selectedPath = buildingEditor->GetSelectedBitmapPath();

		// 경로를 AddBitmap에 전달하여 비트맵 캐싱
		if (!selectedPath.empty())
			AddBitmap(selectedPath);

		// _buildingMap에는 경로를 저장
		if (gridX >= 0 && gridX < GridWidth && gridY >= 0 && gridY < GridHeight)
			_buildingMap[gridY][gridX] = selectedPath;
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::RightMouse))
	{
		if (gridX >= 0 && gridX < GridWidth && gridY >= 0 && gridY < GridHeight)
			_buildingMap[gridY][gridX] = L"";
	}
}

void EditorScene::UpdateEditEnemyMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();

	int worldX = (mousePos.x + cameraPosition.x);
	int worldY = (mousePos.y + cameraPosition.y);

	int gridX = worldX / TileSize;
	int gridY = worldY / TileSize;

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
	{
		if (gridX >= 0 && gridX < GridWidth && gridY >= 0 && gridY < GridHeight)
			_enemySpawnMap[gridY][gridX] = !_enemySpawnMap[gridY][gridX];
	}
}

void EditorScene::UpdateEditPlatformMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();

	int worldX = (mousePos.x + cameraPosition.x);
	int worldY = (mousePos.y + cameraPosition.y);

	int gridX = worldX / TileSize;
	int gridY = worldY / TileSize;

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
	{
		if (gridX >= 0 && gridX < GridWidth && gridY >= 0 && gridY < GridHeight)
			_platformSpawnMap[gridY][gridX] = !_platformSpawnMap[gridY][gridX];
	}
}

void EditorScene::SetEditMode()
{
	if (editmode == EEditMode::Tile)
		editmode = EEditMode::Collision;
	else if (editmode == EEditMode::Collision)
		editmode = EEditMode::Building;
	else if (editmode == EEditMode::Building)
		editmode = EEditMode::Enemy;
	else if (editmode == EEditMode::Enemy)
		editmode = EEditMode::Platform;
	else
		editmode = EEditMode::Tile;
}

int EditorScene::GetSelectedTileIndex()
{
	if (tileEditor)
		return tileEditor->GetSelectedIndex();
	return -1;
}

void EditorScene::DrawMainGrid(HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	// 격자 그리기
	for (int x = 0; x <= GridWidth; x++)
	{
		MoveToEx(hdc, x * TileSize - cameraPosition.x, -cameraPosition.y, NULL);
		LineTo(hdc, x * TileSize - cameraPosition.x, GridHeight * TileSize - cameraPosition.y);
	}

	for (int y = 0; y <= GridHeight; y++)
	{
		MoveToEx(hdc, -cameraPosition.x, y * TileSize - cameraPosition.y, NULL);
		LineTo(hdc, GridWidth * TileSize - cameraPosition.x, y * TileSize - cameraPosition.y);
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	// 타일 그리기
	for (int i = 0; i < _layerCount; i++)
	{
		if (_isDrawCurrLayer && i != _selectedLayer)
			continue;

		// 레이어별 타일 그리기
		for (int y = 0; y < GridHeight; y++)
		{
			for (int x = 0; x < GridWidth; x++)
			{
				int tileIndex = _layer[i].mainGrid[y * GridWidth + x];
				if (tileIndex >= 0)
					DrawTileOnGrid(hdc, i, x, y);
			}
		}
	}
}

void EditorScene::DrawTileOnGrid(HDC hdc, int layer, int gridX, int gridY)
{
	int encodedValue = _layer[layer].mainGrid[gridY * GridWidth + gridX];

	if (encodedValue < 0) return;  // 빈 타일

	TileInfo tileInfo = TileInfo::Decode(encodedValue);

	HDC sourceDC = nullptr;
	if (tileInfo.tilesetType == 0)
		sourceDC = _hdcBitmap;  // Normal 타일셋
	else if (tileInfo.tilesetType == 1)
		sourceDC = _hdcDeathBitmap;  // Death 타일셋

	int tileX = tileInfo.tileIndex % TileMapWidth;
	int tileY = tileInfo.tileIndex / TileMapWidth;

	TransparentBlt(
		hdc,
		gridX * TileSize - cameraPosition.x,
		gridY * TileSize - cameraPosition.y,
		TileSize,
		TileSize,
		sourceDC,  // 선택된 타일셋 HDC 사용
		tileX * OriginTileSize,
		tileY * OriginTileSize,
		OriginTileSize,
		OriginTileSize,
		_transparent
	);

	// Death 타일에 추가 표시 (선택적)
	if (tileInfo.tilesetType == 1)
	{
		// Death 타일에 빨간 테두리 표시 (디버깅용)
		/*
		HPEN redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		HPEN oldPen = (HPEN)SelectObject(hdc, redPen);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

		Rectangle(hdc,
			gridX * TileSize - cameraPosition.x,
			gridY * TileSize - cameraPosition.y,
			(gridX + 1) * TileSize - cameraPosition.x,
			(gridY + 1) * TileSize - cameraPosition.y);

		SelectObject(hdc, oldPen);
		SelectObject(hdc, oldBrush);
		DeleteObject(redPen);
		*/
	}
}

void EditorScene::DrawBuildings(HDC hdc)
{
	for (int y = 0; y < GridHeight; ++y)
	{
		for (int x = 0; x < GridWidth; ++x)
		{
			const wstring& buildingPath = _buildingMap[y][x];
			if (!buildingPath.empty())
			{
				// 맵에서 해당 경로의 HBITMAP을 찾음
				HBITMAP buildingBitmap = _loadedBitmaps[buildingPath];

				if (buildingBitmap) // 비트맵이 유효한지 다시 확인
				{
					HDC hdcMem = ::CreateCompatibleDC(hdc);
					HBITMAP oldBitmap = (HBITMAP)::SelectObject(hdcMem, buildingBitmap);

					BITMAP bmpInfo;
					::GetObject(buildingBitmap, sizeof(BITMAP), &bmpInfo);

					int renderX = x * TileSize - cameraPosition.x;
					int renderY = y * TileSize - cameraPosition.y;

					::TransparentBlt(
						hdc,
						renderX, renderY, bmpInfo.bmWidth, bmpInfo.bmHeight,
						hdcMem, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight,
						RGB(255, 0, 255)
					);

					::SelectObject(hdcMem, oldBitmap);
					::DeleteDC(hdcMem);
				}
			}
		}
	}
}

void EditorScene::DrawEnemySpawns(HDC hdc)
{
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 0, 0)); // 빨간색 브러시
	HPEN hPen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

	for (int y = 0; y < GridHeight; ++y)
	{
		for (int x = 0; x < GridWidth; ++x)
		{
			if (_enemySpawnMap[y][x])
			{
				// 마우스 위치와 상관없이 카메라 위치를 고려하여 그리기
				int renderX = x * TileSize - cameraPosition.x;
				int renderY = y * TileSize - cameraPosition.y;

				// 사각형 그리기 (간단한 시각적 표시)
				::Rectangle(hdc, renderX, renderY, renderX + TileSize, renderY + TileSize);
			}
		}
	}

	::SelectObject(hdc, hOldPen);
	::SelectObject(hdc, hOldBrush);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void EditorScene::DrawPlatformSpawns(HDC hdc)
{
	HBRUSH hBrush = ::CreateSolidBrush(RGB(0, 255, 0)); // 초록색 브러시
	HPEN hPen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

	for (int y = 0; y < GridHeight; ++y)
	{
		for (int x = 0; x < GridWidth; ++x)
		{
			if (_platformSpawnMap[y][x])
			{
				// 마우스 위치와 상관없이 카메라 위치를 고려하여 그리기
				int renderX = x * TileSize - cameraPosition.x;
				int renderY = y * TileSize - cameraPosition.y;

				// 사각형 그리기 (간단한 시각적 표시)
				::Rectangle(hdc, renderX, renderY, renderX + TileSize, renderY + TileSize);
			}
		}
	}

	::SelectObject(hdc, hOldPen);
	::SelectObject(hdc, hOldBrush);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void EditorScene::DrawCollisionRects(HDC hdc)
{
	for (const auto& collision : collisionRects)
	{
		HBRUSH brush = nullptr;
		HPEN pen = nullptr;
		COLORREF color;

		if (collision.type == CollisionType::Normal)
		{
			color = RGB(0, 255, 0);  // 초록색 - 일반 충돌
			brush = CreateSolidBrush(color);
			pen = CreatePen(PS_SOLID, 2, RGB(0, 200, 0));
		}
		else  
		{
			color = RGB(255, 0, 0);  // 빨간색 - 사망 충돌
			brush = CreateSolidBrush(color);
			pen = CreatePen(PS_SOLID, 2, RGB(200, 0, 0));
		}

		SetBkMode(hdc, TRANSPARENT);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);

		RECT screenRect;
		screenRect.left = collision.rect.left - cameraPosition.x;
		screenRect.top = collision.rect.top - cameraPosition.y;
		screenRect.right = collision.rect.right - cameraPosition.x;
		screenRect.bottom = collision.rect.bottom - cameraPosition.y;

		HBRUSH hatchBrush;
		if (collision.type == CollisionType::Normal)
			hatchBrush = CreateHatchBrush(HS_BDIAGONAL, color);  
		else
			hatchBrush = CreateHatchBrush(HS_DIAGCROSS, color);  

		SelectObject(hdc, hatchBrush);
		Rectangle(hdc, screenRect.left, screenRect.top, screenRect.right, screenRect.bottom);

		SelectObject(hdc, oldBrush);
		SelectObject(hdc, oldPen);
		DeleteObject(brush);
		DeleteObject(pen);
		DeleteObject(hatchBrush);
	}

	if (_isDragging)
	{
		COLORREF previewColor = bOnDeathTile ? RGB(255, 100, 100) : RGB(100, 255, 100);
		HPEN previewPen = CreatePen(PS_DOT, 2, previewColor);
		HPEN oldPen = (HPEN)SelectObject(hdc, previewPen);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

		int left = min(dragStartPos.x, dragEndPos.x) * TileSize - cameraPosition.x;
		int top = min(dragStartPos.y, dragEndPos.y) * TileSize - cameraPosition.y;
		int right = (max(dragStartPos.x, dragEndPos.x) + 1) * TileSize - cameraPosition.x;
		int bottom = (max(dragStartPos.y, dragEndPos.y) + 1) * TileSize - cameraPosition.y;

		Rectangle(hdc, left, top, right, bottom);

		SelectObject(hdc, oldPen);
		SelectObject(hdc, oldBrush);
		DeleteObject(previewPen);
	}
}

void EditorScene::SaveTileMap()
{
	OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mainWnd;
	ofn.lpstrFilter = L"타일맵 파일 (*.tilemap)\0*.tilemap\0모든 파일 (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"tilemap";

	if (GetSaveFileName(&ofn))
	{
		// 파일 이름이 선택되었으면 저장
		wstring wFileName = szFileName;
		wstring fileName(wFileName.begin(), wFileName.end());

		wofstream file(fileName);
		if (file.is_open())
		{
			// 그리드 크기 저장
			file << GridWidth << "," << GridHeight << "," << TileMapWidth << "," << TileMapHeight << endl;

			// 타일 데이터 저장
			for (int i = 0; i < _layerCount; i++)
			{
				int size = _layer[i].GetValidCount();
				file << i << ":" << size << std::endl;

				for (int y = 0; y < GridHeight; y++)
				{
					for (int x = 0; x < GridWidth; x++)
					{
						int encodedValue = _layer[i].mainGrid[y * GridWidth + x];
						if (encodedValue >= 0)
							file << x << "," << y << "," << encodedValue << std::endl;
					}
				}
			}

			file << L"[Collision]" << endl;
			file << collisionRects.size() << endl;
			for (const auto& collision : collisionRects)
			{
				file << collision.rect.left << "," << collision.rect.top << ","
					<< collision.rect.right << "," << collision.rect.bottom << ","
					<< static_cast<int>(collision.type) << endl;
			}

			file << L"[Buildings]" << endl;
			for (int y = 0; y < GridHeight; y++)
			{
				for (int x = 0; x < GridWidth; x++)
				{
					const wstring& buildingPath = _buildingMap[y][x];
					if (!buildingPath.empty())
						file << x << "," << y << "," << buildingPath << endl;
				}
			}

			file << L"[Enemies]" << endl;
			for (int y = 0; y < GridHeight; ++y)
			{
				for (int x = 0; x < GridWidth; ++x)
				{
					if (_enemySpawnMap[y][x])
						file << x << "," << y << endl;
				}
			}
			file << L"[Platforms]" << endl;
			for (int y = 0; y < GridHeight; ++y)
			{
				for (int x = 0; x < GridWidth; ++x)
				{
					if (_platformSpawnMap[y][x])
						file << x << "," << y << endl;
				}
			}

			file.close();
			MessageBox(mainWnd, L"타일맵이 저장되었습니다.", L"저장 완료", MB_OK | MB_ICONINFORMATION);
		}
		else 
			MessageBox(mainWnd, L"파일을 저장할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);

	}
}

void EditorScene::LoadTileMap()
{
	OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mainWnd;
	ofn.lpstrFilter = L"타일맵 파일 (*.tilemap)\0*.tilemap\0모든 파일 (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"tilemap";

	if (GetOpenFileName(&ofn))
	{
		wstring wFileName = szFileName;
		wstring fileName(wFileName.begin(), wFileName.end());

		wifstream file(fileName);
		if (file.is_open())
		{
			// 충돌 영역 초기화
			collisionRects.clear();
			_buildingMap.assign(GridHeight, std::vector<std::wstring>(GridWidth, L"")); // 건축물 맵도 초기화

			wchar_t comma;
			int width, height, tileMapW, tileMapH;
			file >> width >> comma >> height >> comma >> tileMapW >> comma >> tileMapH;

			if (width != GridWidth || height != GridHeight)
			{
				MessageBox(mainWnd,
					L"로드된 타일맵의 크기가 현재 그리드와 다릅니다. 일부 타일이 잘릴 수 있습니다.",
					L"경고", MB_OK | MB_ICONWARNING);
			}
				
			for (int i = 0; i < _layerCount; i++)
				_layer[i].mainGrid.assign(GridWidth * GridHeight, -1);

			wstring line;
			int currentSection = 0; // 0: Tiles, 1: Collision, 2: Buildings

			while (getline(file, line))
			{
				if (line == L"[Collision]")
				{
					currentSection = 1;
					continue; // 다음 줄부터 충돌 데이터
				}
				if (line == L"[Buildings]")
				{
					currentSection = 2;
					continue; // 다음 줄부터 건물 데이터
				}
				if (line == L"[Enemies]")
				{
					currentSection = 3;
					continue; 
				}
				if (line == L"[Platforms]")
				{
					currentSection = 4;
					continue; 
				}

				if (currentSection == 0) // 타일 데이터 처리
				{
					wistringstream iss(line);
					int layerIndex, tileCount;
					wchar_t colon;

					if (iss >> layerIndex >> colon >> tileCount)
					{
						if (layerIndex < 0 || layerIndex >= _layerCount)
						{
							MessageBox(mainWnd, L"잘못된 레이어 데이터가 발견되었습니다.", L"오류", MB_OK | MB_ICONERROR);
							break;
						}

						for (int index = 0; index < tileCount; ++index)
						{
							getline(file, line);
							if (!line.empty())
							{
								wistringstream tileStream(line);
								int x, y, tileIndex;
								wchar_t comma;

								if (tileStream >> x >> comma >> y >> comma >> tileIndex)
								{
									if (x >= 0 && x < GridWidth && y >= 0 && y < GridHeight)
										_layer[layerIndex].mainGrid[y * GridWidth + x] = tileIndex;
								}
							}
						}
					}
				}
				else if (currentSection == 1) // 충돌 데이터 처리
				{
					int collisionCount = _wtoi(line.c_str());
					for (int i = 0; i < collisionCount; i++)
					{
						if (getline(file, line))
						{
							wistringstream rectStream(line);
							RECT rect;
							int typeValue = 0;  // 기본값은 Normal
							wchar_t comma;

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
									collisionRects.push_back(CollisionRect(rect, CollisionType::Normal));
							}
						}
					}
				}
				else if (currentSection == 2) // 건물 데이터 처리
				{
					wistringstream buildingStream(line);
					int x, y;
					wchar_t comma;
					std::wstring path;

					if (buildingStream >> x >> comma >> y >> comma)
					{
						getline(buildingStream, path);
						if (x >= 0 && x < GridWidth && y >= 0 && y < GridHeight)
						{
							_buildingMap[y][x] = path;
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
						if (x >= 0 && x < GridWidth && y >= 0 && y < GridHeight)
						{
							_enemySpawnMap[y][x] = true;
						}
					}
				}
				else if (currentSection == 4) // 적군 데이터 처리
				{
					wistringstream enemyStream(line);
					int x, y;
					wchar_t comma;

					if (enemyStream >> x >> comma >> y)
					{
						if (x >= 0 && x < GridWidth && y >= 0 && y < GridHeight)
						{
							_platformSpawnMap[y][x] = true;
						}
					}
				}
			}

			file.close();

			// 비트맵 캐싱
			for (const auto& row : _buildingMap)
			{
				for (const auto& path : row)
				{
					if (!path.empty())
						AddBitmap(path);
				}
			}
			MessageBox(mainWnd, L"타일맵이 로드되었습니다.", L"로드 완료", MB_OK | MB_ICONINFORMATION);
		}
		else
			MessageBox(mainWnd, L"파일을 로드할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
	}
}

HBITMAP EditorScene::AddBitmap(const wstring& filePath)
{
	if (_loadedBitmaps.count(filePath) > 0)
		return _loadedBitmaps[filePath];

	// 새로운 비트맵을 로드
	HBITMAP hBitmap = (HBITMAP)::LoadImageW(
		nullptr,
		filePath.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE
	);

	if (hBitmap)
	{
		// 로드 성공 시 맵에 저장
		_loadedBitmaps[filePath] = hBitmap;
	}

	return hBitmap;
}

int EditorScene::TileLayer::GetValidCount()
{
	int count = 0;
	for (auto iter : mainGrid)
	{
		if (iter >= 0) count++;
	}
	return count;
}