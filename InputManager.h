#pragma once

#include "Singleton.h"

enum class KeyState
{
	None,
	Press,		// 누르고 있는 중
	Down,		// 방금 누름
	Up,			// 방금 뗏음

	End
};

enum class KeyType
{
	LeftMouse = VK_LBUTTON,
	RightMouse = VK_RBUTTON,
	SpaceBar = VK_SPACE,
	Shift = VK_SHIFT,

	A = 'A',
	D = 'D',
	W = 'W',
	S = 'S',
	T = 'T',
	L = 'L',
	Q = 'Q',
	E = 'E',
	Tab = VK_TAB
};

constexpr int32 KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX) + 1;

class InputManager : public Singleton<InputManager>
{
	friend Singleton<InputManager>;

private:
	InputManager() {}

public:
	void Init(HWND hwnd, HWND subwnd, HWND subwnd2);
	void Update();

	bool GetButtonPressed(KeyType key) { return GetState(key) == KeyState::Press; }
	bool GetButtonDown(KeyType key) { return GetState(key) == KeyState::Down; }
	bool GetButtonUp(KeyType key) { return GetState(key) == KeyState::Up; }
	int GetMouseWheelDelta() const { return _mouseWheelDelta; }
	void SetMouseWheelDelta(int delta) { _mouseWheelDelta = delta; }
	
	POINT GetMousePos() { return _mousePos; }
	POINT GetSubMousePos() { return _subMousePos; }
	POINT GetSub2MousePos() { return _sub2MousePos; }
private:
	KeyState GetState(KeyType key) { return _states[static_cast<uint8>(key)]; }

private:
	HWND _hwnd = 0;	
	HWND _subwnd = 0;
	HWND _subwnd2 = 0;
	vector<KeyState> _states;
	POINT _mousePos = {};
	POINT _subMousePos = {};
	POINT _sub2MousePos = {};
	int _mouseWheelDelta = 0;
};

