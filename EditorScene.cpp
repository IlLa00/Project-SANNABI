#include "pch.h"
#include "EditorScene.h"
#include "TileEditor.h"
#include "InputManager.h"
#include "CameraManager.h"

void EditorScene::Init()
{
	tileEditor = new TileEditor();
	tileEditor->Init(subWnd);

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

	for (int i = 0; i < _layerCount; i++)
		_layer[i].mainGrid.resize(GridWidth * GridHeight, -1);

	ShowWindow(subWnd, true);
	UpdateWindow(subWnd);
}

void EditorScene::Update(float deltaTime)
{
	HWND hwnd = ::GetForegroundWindow();

	if (hwnd == subWnd && subWnd)
	{
		tileEditor->Update();
		return;
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::LeftMouse))
		_isDragging = true;

	if (editmode == EEditMode::Tile)
		UpdateEditTileMode();
	else
		UpdateEditCollisionMode();

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
	tileEditor->Render();

	DrawMainGrid(_hdcBack); // 백 버퍼에 그리기
	DrawCollisionRects(_hdcBack);

	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	int x = mousePos.x / TileSize;
	int y = mousePos.y / TileSize;

	{
		const wchar_t* modestr;
		if(editmode == EEditMode::Tile)
			modestr = L"Tile";
		else
			modestr = L"Collision";

		std::wstring str = std::format(L"Layer:{0}, State:{1}", _selectedLayer, modestr);
		::TextOut(_hdcBack, 5, 10, str.c_str(), static_cast<int>(str.size()));
	}

	{
		std::wstring str = std::format(L"x:{0}, y:{1}", x, y);
		::TextOut(_hdcBack, 5, 30, str.c_str(), static_cast<int>(str.size()));
	}
}

void EditorScene::Destroy()
{

}

void EditorScene::UpdateEditTileMode()
{
	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	// 클릭한 그리드 위치 계산
	int worldX = (mousePos.x + cameraPosition.x) / TileSize;
	int worldY = (mousePos.y + cameraPosition.y) / TileSize;

	if (_isDragging && InputManager::GetInstance()->GetButtonPressed(KeyType::LeftMouse))
	{
		// 그리드 범위 확인
		if (worldX >= 0 && worldX < GridWidth && worldY >= 0 && worldY < GridHeight && GetSelectedTileIndex() >= 0)
			_layer[_selectedLayer].mainGrid[worldY * GridWidth + worldX] = GetSelectedTileIndex();
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::RightMouse))
	{
		POINT mousePos = InputManager::GetInstance()->GetMousePos();
		// 클릭한 그리드 위치 계산
		int worldX = (mousePos.x + cameraPosition.x) / TileSize;
		int worldY = (mousePos.y + cameraPosition.y) / TileSize;

		// 그리드 범위 확인
		if (worldX >= 0 && worldX < GridWidth && worldY >= 0 && worldY < GridHeight && GetSelectedTileIndex() >= 0)
		{
			// 선택된 타일을 그리드에 배치
			_layer[_selectedLayer].mainGrid[worldY * GridWidth + worldX] = -1;
		}
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

	// 마우스 왼쪽 버튼 뗌 - 충돌 영역 생성
	if (InputManager::GetInstance()->GetButtonUp(KeyType::LeftMouse))
	{
		// 직사각형 생성 (좌상단, 우하단 정렬)
		RECT collisionRect;
		collisionRect.left = min(dragStartPos.x, dragEndPos.x) * TileSize;
		collisionRect.top = min(dragStartPos.y, dragEndPos.y) * TileSize;
		collisionRect.right = (max(dragStartPos.x, dragEndPos.x)+1) * TileSize;
		collisionRect.bottom = (max(dragStartPos.y, dragEndPos.y)+1) * TileSize;

		collisionRects.push_back(collisionRect);
	}

	if (InputManager::GetInstance()->GetButtonDown(KeyType::RightMouse))
	{
		// 클릭한 위치와 겹치는 충돌 영역 찾아서 삭제
		auto it = remove_if(collisionRects.begin(), collisionRects.end(),
			[worldX, worldY](const RECT& rect)
			{
				return worldX >= rect.left && worldX < rect.right &&
					worldY >= rect.top && worldY < rect.bottom;
			});
		collisionRects.erase(it, collisionRects.end());
	}
}

void EditorScene::SetEditMode()
{
	if (editmode == EEditMode::Tile)
		editmode = EEditMode::Collision;
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
	int tileIndex = _layer[layer].mainGrid[gridY * GridWidth + gridX];

	// 선택한 타일을 그리드에 그리기
	int tileX = tileIndex % TileMapWidth;
	int tileY = tileIndex / TileMapWidth;

	// 흰색(RGB(255, 255, 255))을 투명색으로 처리하여 비트맵 그리기
	TransparentBlt(
		hdc,
		gridX * TileSize - cameraPosition.x,
		gridY * TileSize - cameraPosition.y,
		TileSize,
		TileSize, // 대상 위치 및 크기
		_hdcBitmap,
		tileX * OriginTileSize, tileY * OriginTileSize, OriginTileSize, OriginTileSize, // 원본 위치 및 크기
		_transparent
	);
}

void EditorScene::DrawCollisionRects(HDC hdc)
{
	// 저장된 충돌 영역들 그리기
	for (const auto& collision : collisionRects)
	{
		HBRUSH brush = nullptr;
		HPEN pen = nullptr;

		brush = CreateSolidBrush(RGB(255, 0, 0));  // 빨간색
		pen = CreatePen(PS_SOLID, 2, RGB(200, 0, 0));

		// 반투명 효과를 위해 패턴 브러시 사용
		SetBkMode(hdc, TRANSPARENT);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);

		// 화면 좌표로 변환
		RECT screenRect;
		screenRect.left = collision.left - cameraPosition.x;
		screenRect.top = collision.top - cameraPosition.y;
		screenRect.right = collision.right - cameraPosition.x;
		screenRect.bottom = collision.bottom - cameraPosition.y;

		// 투명도 50%로 그리기 (알파 블렌딩 대신 해칭 패턴)
		HBRUSH hatchBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(255, 0, 0));

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
		HPEN previewPen = CreatePen(PS_DOT, 1, RGB(100, 100, 255));
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
						int tileIndex = _layer[i].mainGrid[y * GridWidth + x];
						if (tileIndex >= 0)
						{
							file << x << "," << y << "," << tileIndex << std::endl;
						}
					}
				}
			}

			file << L"[Collision]" << endl;
			file << collisionRects.size() << endl;
			for (const auto& rect : collisionRects)
			{
				// 픽셀 좌표로 저장 (left,top,right,bottom)
				file << rect.left << "," << rect.top << ","
					<< rect.right << "," << rect.bottom << endl;
			}

			file.close();
			MessageBox(mainWnd, L"타일맵이 저장되었습니다.", L"저장 완료", MB_OK | MB_ICONINFORMATION);
		}
		else {
			MessageBox(mainWnd, L"파일을 저장할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
		}
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
		// 파일 이름이 선택되었으면 로드
		wstring wFileName = szFileName;
		wstring fileName(wFileName.begin(), wFileName.end());

		wifstream file(fileName);
		if (file.is_open())
		{
			// 충돌 영역 초기화
			collisionRects.clear();

			wchar_t comma;

			// 그리드 크기 로드
			int width, height, tileMapW, tileMapH;
			file >> width >> comma >> height >> comma >> tileMapW >> comma >> tileMapH;

			// 기존 그리드 크기와 다르면 경고
			if (width != GridWidth || height != GridHeight)
			{
				MessageBox(mainWnd,
					L"로드된 타일맵의 크기가 현재 그리드와 다릅니다. 일부 타일이 잘릴 수 있습니다.",
					L"경고", MB_OK | MB_ICONWARNING);
			}

			// 레이어 데이터 초기화
			for (int i = 0; i < _layerCount; i++)
				_layer[i].mainGrid.assign(GridWidth * GridHeight, -1);

			// 레이어별 데이터 로드
			wstring line;
			while (getline(file, line))
			{
				// [Collision] 섹션을 만나면 충돌 데이터 처리
				if (line == L"[Collision]")
				{
					// 충돌 영역 개수 읽기
					getline(file, line);
					int collisionCount = _wtoi(line.c_str());

					// 충돌 영역들 로드
					for (int i = 0; i < collisionCount; i++)
					{
						if (getline(file, line))
						{
							wistringstream rectStream(line);
							RECT rect;
							wchar_t comma;

							// left,top,right,bottom 형식으로 읽기
							if (rectStream >> rect.left >> comma >> rect.top >> comma
								>> rect.right >> comma >> rect.bottom)
							{
								collisionRects.push_back(rect);
							}
						}
					}
					break;  // 충돌 데이터 읽기 완료
				}

				// 타일 레이어 데이터 처리
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

					// 타일 데이터 읽기
					for (int index = 0; index < tileCount; ++index)
					{
						getline(file, line);

						if (line.empty() == false)
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

			file.close();
			MessageBox(mainWnd, L"타일맵이 로드되었습니다.", L"로드 완료", MB_OK | MB_ICONINFORMATION);
		}
		else
			MessageBox(mainWnd, L"파일을 로드할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
	}
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