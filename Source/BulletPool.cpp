#include "pch.h"
#include "BulletPool.h"
#include "Bullet.h"
#include "CollisionComponent.h"

void BulletPool::CreatePool(int size)
{
    for (int i = 0; i < size; i++)
    {
        Bullet* bullet = new Bullet();
        bullet->Deactivate();
        bullet->Init();
        CollisionComponent* collisionComp = bullet->GetComponent<CollisionComponent>();
        if (!collisionComp) continue;

        collisionComp->OnComponentBeginOverlap = [this, bullet](CollisionComponent* other, HitResult info)
            {
                if (other->GetOwner() == bullet->GetOwner()) return;

                ReturnProjectile(bullet);
            };
        pool.push_back(bullet);
    }
}

void BulletPool::Destroy()
{
    for (auto& bullet : pool)
        SAFE_DELETE(bullet);

    pool.clear();
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