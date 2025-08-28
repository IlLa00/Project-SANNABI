#include "pch.h"
#include "InputManager.h"

void InputManager::Init(HWND hwnd, HWND subwnd, HWND subwnd2)
{
	_hwnd = hwnd;
	_subwnd = subwnd;
	_subwnd2 = subwnd2;

	_states.resize(KEY_TYPE_COUNT, KeyState::None);	
}

void InputManager::Update()
{
	if (bEndGame)
		return;

	BYTE asciiKeys[KEY_TYPE_COUNT] = {};
	if (::GetKeyboardState(asciiKeys) == false)
		return;

	for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
	{
		if (asciiKeys[key] & 0x80)
		{
			KeyState& state = _states[key];

			if (state == KeyState::Press || state == KeyState::Down)
				state = KeyState::Press;
			else
				state = KeyState::Down;
		}
		else
		{
			KeyState& state = _states[key];

			if (state == KeyState::Press || state == KeyState::Down)
				state = KeyState::Up;
			else
				state = KeyState::None;
		}
	}

	_mouseWheelDelta = 0;

	::GetCursorPos(&_mousePos); 
	::ScreenToClient(_hwnd, &_mousePos);	

	::GetCursorPos(&_subMousePos); // 커서의 좌표를 알아온다
	::ScreenToClient(_subwnd, &_subMousePos);

	::GetCursorPos(&_sub2MousePos); // 커서의 좌표를 알아온다
	::ScreenToClient(_subwnd, &_sub2MousePos);
}
