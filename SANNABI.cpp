#include "pch.h"
#include "SANNABI.h"
#include <windowsx.h>
#include <format>
#include "Engine.h"
#include "SceneManager.h"
#include "EditorScene.h"
#include "InputManager.h"
#include "BuildingEditor.h"

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];             // the main window class name
HINSTANCE hInst; 
HWND gHwnd, gSubWnd, gSub2Wnd;

Engine* engine;

ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                RegisterSubWindowClass(HINSTANCE hInstance);
ATOM                RegisterSub2WindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    SubWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    Sub2WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SANNABI, szWindowClass, MAX_LOADSTRING);
    
    MyRegisterClass(hInstance);
    RegisterSubWindowClass(hInstance);
    RegisterSub2WindowClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SANNABI));

    MSG msg = {};

    // 엔진 초기화
    engine = Engine::GetInstance();  // 변수 선언 주소 : 0x01
    engine->Init(gHwnd, gSubWnd, gSub2Wnd);

    InputManager::GetInstance()->Init(gHwnd, gSubWnd, gSub2Wnd);

    LARGE_INTEGER frequency, now, prev;
    ::QueryPerformanceFrequency(&frequency);
    ::QueryPerformanceCounter(&prev);

    const float targetFrameTime = 1000.0f / 120.f;

    // Main message loop:
    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ::QueryPerformanceCounter(&now);
            float elapsed = (now.QuadPart - prev.QuadPart) / static_cast<float>(frequency.QuadPart) * 1000.0f;

            //if (elapsed >= targetFrameTime)
            {
                InputManager::GetInstance()->Update();
                engine->Update();
                engine->Render();

                prev = now;
            }
        }
    }

    engine->Destroy();
    engine->DestroyInstance();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SANNABI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = nullptr;//MAKEINTRESOURCEW(IDC_SANNABI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM RegisterSubWindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = SubWndProc; // 서브 메세지 처리
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SubWindowClass";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}

ATOM RegisterSub2WindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Sub2WndProc; // 서브 메세지 처리
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Sub2WindowClass";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT windowRect = { 0, 0, GWinSizeX, GWinSizeY };
   ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, true);

   // 실제로 윈도우창을 생성한다.
   // PC에 여러개의 윈도우창이 떠있을수 있는데 각 윈도우창을 구분하는 핸들
   gHwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

   if (!gHwnd)
   {
      return FALSE;
   }

   ShowWindow(gHwnd, nCmdShow);
   UpdateWindow(gHwnd);

   gSubWnd = CreateWindowW(
       L"SubWindowClass", L"타일 에디터 - 타일 선택",
       WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, TileMapWidth * OriginTileSize + 100, TileMapHeight * OriginTileSize + 100,
       nullptr, nullptr, hInst, nullptr);

   if (!gSubWnd)
   {
       return FALSE;
   }

   gSub2Wnd = CreateWindowW(
       L"Sub2WindowClass", L"건축물 에디터 - 건축물 선택",
       WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
       CW_USEDEFAULT, 0, 1000, 1000,
       nullptr, 
       LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)), // 이 부분이 핵심, 
       hInst, nullptr);

   if (!gSub2Wnd)
   {
       return FALSE;
   }

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK SubWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        // wParam의 HIWORD에 마우스 휠 델타 값이 저장되어 있습니다.
        short delta = HIWORD(wParam);
        InputManager::GetInstance()->SetMouseWheelDelta(delta);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Sub2WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_COMMAND:
    { // 메뉴 항목 클릭 시 이 메시지가 발생
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_FILE_LOAD:
        {
            OPENFILENAME ofn;
            wchar_t szFile[MAX_PATH] = { 0, };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE)
            {
               filesystem::path currentPath = filesystem::current_path();
               filesystem::path selectedPath(szFile);
               filesystem::path relativePath = filesystem::relative(selectedPath, currentPath);

               engine->GetBuildingEditor()->SetSelectedBitmapPath(relativePath.wstring());
            }

            break;
        }
        }
    }
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        // wParam의 HIWORD에 마우스 휠 델타 값이 저장되어 있습니다.
        short delta = HIWORD(wParam);
        InputManager::GetInstance()->SetMouseWheelDelta(delta);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}