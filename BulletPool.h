#pragma once

#include "Singleton.h"

class Bullet;

class BulletPool : public Singleton<BulletPool>
{
	friend Singleton<BulletPool>;

public:
	void CreatePool(int size);
	void Destroy();
	Bullet* GetProjectile(Vector postion, Vector direction, float speed);
	void ReturnProjectile(Bullet* bullet);

private:
	BulletPool() {}

private:
	vector<Bullet*> pool;

};

