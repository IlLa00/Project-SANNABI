#include "pch.h"
#include "Component.h"

void Component::Init(Actor* _owner)
{
	SetOwner(_owner);
}

void Component::Update(float deltaTime)
{
	if (!bActive) return;
}

void Component::Render(HDC _hdcBack)
{

}

void Component::Destroy()
{

}

void Component::SetOwner(Actor* _owner)
{
	if (_owner)
		owner = _owner;
}
