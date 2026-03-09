#pragma once

#include "Singleton.h"

class GrapplingHookProjectile;

class GrapplingHookProjectilePool : public Singleton<GrapplingHookProjectilePool>
{
	friend Singleton<GrapplingHookProjectilePool>;

public:
	void CreatePool(int size);
	void Destroy();
	GrapplingHookProjectile* GetProjectile(Vector postion, Vector direction, float speed);
	void ReturnProjectile(GrapplingHookProjectile* projectile);


private:
	GrapplingHookProjectilePool() {}

private:
	vector<GrapplingHookProjectile*> pool;
};

