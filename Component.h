#pragma once

class Actor;

class Component
{
public:
	virtual void Init(Actor* _owner);
	virtual void Update(float deltaTime);
	virtual void Render(HDC _hdcBack);
	virtual void Destroy();

	void SetOwner(Actor* _owner);
	Actor* GetOwner() { return owner; }

	void SetActive(bool state) { bActive = state; }
	bool IsActive() { return bActive; }

protected:
	Actor* owner;

	bool bActive = true;
};

