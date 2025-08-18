#pragma once

class Component;

class Actor
{
public:
	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Render(HDC _hdcBack);
	virtual void Destroy();

	virtual void TakeDamage();

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
	Vector GetDirection() { return direction; }
	float GetSpeed() { return speed; }

	Actor* GetOwner() { return owner; }
	void SetOwner(Actor* newOwner) { owner = newOwner; }

protected:
	Vector position;
	Vector velocity;
	Vector acceleration;

	float speed = 300.f;
	Vector direction = Vector(1,0);
	float scale = 0.25f;

	Actor* owner = nullptr;

	unordered_set<Component*> components;
};

