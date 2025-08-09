#include "pch.h"
#include "GrapplingHookProjectilePool.h"
#include "GrapplingHookProjectile.h"
#include "Actor.h"

void GrapplingHookProjectilePool::CreatePool(int size)
{
    for (int i = 0; i < size; i++)
    {
        GrapplingHookProjectile* projectile = new GrapplingHookProjectile();
        projectile->Init();
        projectile->Deactivate();
        pool.push_back(projectile);
    }
}

GrapplingHookProjectile* GrapplingHookProjectilePool::GetProjectile(Vector postion, Vector direction, float speed)
{
    for (auto projectile : pool)
    {
        if (!projectile->IsActive())
        {
            projectile->Activate(postion, direction, speed);
            return projectile;
        }
    }

    return nullptr;
}

void GrapplingHookProjectilePool::ReturnProjectile(GrapplingHookProjectile* projectile)
{
    if (projectile)
        projectile->Deactivate();
}
