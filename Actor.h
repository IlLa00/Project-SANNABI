#pragma once

#include "CollisionManager.h"

class CollisionComponent;
class Component;

class Actor
{
public:
	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Render(HDC _hdcBack);
	virtual void Destroy();

	virtual void TakeDamage();

	virtual void OnCharacterBeginOverlap(CollisionComponent* other, HitResult info);

	void AddComponent(Component* component);

	template<typename T>
	T* GetComponent()
	{
		for (Component* comp : components)
		{
			T* castedComp = dynamic_cast<T*>(comp);
			if (castedComp)
			{
				return castedComp;
			}
		}
		return nullptr;
	}

	unordered_set<Component*> GetComponents() { return components; }

	void SetPosition(Vector newPosition) { position = newPosition; }
	Vector GetPosition() { return position; }

	void SetVelocity(Vector newVelocity) { velocity = newVelocity; }
	Vector GetVelocity() { return velocity; }

	void SetAcceleration(Vector newAcceleration) { acceleration = newAcceleration; }
	Vector GetAcceleration() { return acceleration; }
	
	float GetScale() { return scale; }
	
	void SetDirection(Vector newDirection)
	{
		direction = newDirection;
		lastDirection = direction;
	}

	Vector GetDirection() 
	{
		if (direction.x != 0 || direction.y != 0)
			return direction;
		else
			return lastDirection;
	}
	float GetSpeed() { return speed; }

	Actor* GetOwner() { return owner; }
	void SetOwner(Actor* newOwner) { owner = newOwner; }

	bool IsActive() { return bActive; }
	void SetActive(bool newActive) { bActive = newActive; }

protected:
	Vector position;
	Vector velocity;
	Vector acceleration;

	float speed = 600.f;

	Vector direction = Vector(1,0);
	Vector lastDirection = Vector(1, 0);

	float scale = 0.25f;

	Actor* owner = nullptr;

	unordered_set<Component*> components;

	bool bActive = true;
};

