#pragma once

class Scene
{
public:
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(HDC _hdcBack) = 0;
	virtual void Destroy() = 0;
};

