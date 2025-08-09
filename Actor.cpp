#include "pch.h"
#include "Actor.h"
#include "Component.h"

void Actor::Init()
{
	
}

void Actor::Update(float deltaTime)
{
	for (const auto& comp : components)
	{
		if (comp && comp->IsActive())
			comp->Update(deltaTime);
	}
}

void Actor::Render(HDC _hdcBack)
{
	for (const auto& comp : components)
	{
		if (comp && comp->IsActive())
			comp->Render(_hdcBack);
	}
}

void Actor::Destroy()
{
	for (const auto& comp : components)
	{
		if (comp)
			comp->Destroy();
	}
}

void Actor::AddComponent(Component* component)
{
	if(component)
		components.insert(component);
}
