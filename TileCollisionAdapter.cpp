#include "pch.h"
#include "TileCollisionAdapter.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "CollisionComponent.h"

void TileCollisionAdapter::LoadFromTileMap(const vector<CollisionRect>& Collisions)
{
    for (const auto& collision : Collisions)
    {
        CollisionComponent* collisionComponent = new CollisionComponent();
        collisionComponent->Init(nullptr);

        float centerX = collision.rect.left + (collision.rect.right - collision.rect.left) / 2.0f;
        float centerY = collision.rect.top + (collision.rect.bottom - collision.rect.top) / 2.0f;

        collisionComponent->SetPosition(Vector(centerX, centerY));
        collisionComponent->SetCollisionSize((collision.rect.right - collision.rect.left) / 2, (collision.rect.bottom - collision.rect.top) / 2);

        if (collision.type == CollisionType::Normal)
            collisionComponent->SetCollisionChannel(ECollisionChannel::WorldStatic);
        else
            collisionComponent->SetCollisionChannel(ECollisionChannel::DeathTile);

        CollisionManager::GetInstance()->RegisterCollisionComponent(collisionComponent);
        staticCollisions.push_back(collisionComponent);
    }
}

void TileCollisionAdapter::Render(HDC hdc)
{
    for (const auto& comp : staticCollisions)
        comp->Render(hdc);
}
