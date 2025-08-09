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
	S = 'S'
};

constexpr int32 KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX) + 1;

class InputManager : public Singleton<InputManager>
{
	friend Singleton<InputManager>;

private:
	InputManager() {}

public:

	void Init(HWND hwnd);
	void Update();

	bool GetButtonPressed(KeyType key) { return GetState(key) == KeyState::Press; }
	bool GetButtonDown(KeyType key) { return GetState(key) == KeyState::Down; }
	bool GetButtonUp(KeyType key) { return GetState(key) == KeyState::Up; }

	POINT GetMousePos() { return _mousePos; }

private:
	KeyState GetState(KeyType key) { return _states[static_cast<uint8>(key)]; }

private:
	HWND _hwnd = 0;	
	vector<KeyState> _states;
	POINT _mousePos = {};
};

