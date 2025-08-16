#include "pch.h"
#include "BulletPool.h"
#include "Bullet.h"

void BulletPool::CreatePool(int size)
{
    for (int i = 0; i < size; i++)
    {
        Bullet* bullet = new Bullet();
        bullet->Init();
        bullet->Deactivate();
        pool.push_back(bullet);
    }
}

Bullet* BulletPool::GetProjectile(Vector postion, Vector direction, float speed)
{
    for (auto bullet : pool)
    {
        if (!bullet->IsActive())
        {
            bullet->Activate(postion, direction, speed);
            return bullet;
        }
    }

    return nullptr;
}

void BulletPool::ReturnProjectile(Bullet* bullet)
{
    if (bullet)
        bullet->Deactivate();
}