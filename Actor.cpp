#include "pch.h"
#include "Actor.h"
#include "Component.h"

void Actor::Init()
{
	
}

void Actor::Update(float deltaTime)
{
	if (!bActive) return;

	for (const auto& comp : components)
	{
		if (comp && comp->IsActive())
			comp->Update(deltaTime);
	}
}

void Actor::Render(HDC _hdcBack)
{
	if (!bActive) return;

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

void Actor::TakeDamage()
{

}

void Actor::OnCharacterBeginOverlap(CollisionComponent* other, HitResult info)
{

}

void Actor::AddComponent(Component* component)
{
	if (!bActive) return;

	if(component)
		components.insert(component);
}
