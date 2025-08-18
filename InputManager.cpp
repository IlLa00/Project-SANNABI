#include "pch.h"
#include "InputManager.h"

void InputManager::Init(HWND hwnd, HWND subwnd)
{
	_hwnd = hwnd;
	_subwnd = subwnd;
	_states.resize(KEY_TYPE_COUNT, KeyState::None);	
}

void InputManager::Update()
{
	BYTE asciiKeys[KEY_TYPE_COUNT] = {};
	if (::GetKeyboardState(asciiKeys) == false)
		return;

	for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
	{
		if (asciiKeys[key] & 0x80)
		{
			KeyState& state = _states[key];

			if (state == KeyState::Press || state == KeyState::Down)
			{
				state = KeyState::Press;

				if (key == (int32)KeyType::SpaceBar)
				{
					//OutputDebugStringA("Press SpaceBar\n");
				}
			}
			else
			{
				state = KeyState::Down;
				if (key == (int32)KeyType::SpaceBar)
				{
					OutputDebugStringA("Down SpaceBar\n");
				}

			}
		}
		else
		{
			KeyState& state = _states[key];

			if (state == KeyState::Press || state == KeyState::Down)
			{
				state = KeyState::Up;
				if (key == (int32)KeyType::SpaceBar)
				{
					OutputDebugStringA("Up SpaceBar\n");
				}
			}
			else
			{
				state = KeyState::None;
				if (key == (int32)KeyType::SpaceBar)
				{
					//OutputDebugStringA("None SpaceBar");
				}
			}
		}
	}

	_mouseWheelDelta = 0;

	::GetCursorPos(&_mousePos); 
	::ScreenToClient(_hwnd, &_mousePos);	

	::GetCursorPos(&_subMousePos); // 커서의 좌표를 알아온다
	::ScreenToClient(_subwnd, &_subMousePos);
}
