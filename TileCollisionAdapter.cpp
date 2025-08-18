#include "pch.h"
#include "TileCollisionAdapter.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "CollisionComponent.h"

void TileCollisionAdapter::LoadFromTileMap(const vector<RECT>& Collisions)
{
    for (const auto& rect : Collisions)
    {
        CollisionComponent* collisionComponent = new CollisionComponent();
        collisionComponent->Init(nullptr);

        float centerX = rect.left + (rect.right - rect.left) / 2.0f;
        float centerY = rect.top + (rect.bottom - rect.top) / 2.0f;

        collisionComponent->SetPosition(Vector(centerX, centerY));
        collisionComponent->SetCollisionSize((rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2);

        collisionComponent->SetCollisionChannel(ECollisionChannel::WorldStatic);
        CollisionManager::GetInstance()->RegisterCollisionComponent(collisionComponent);
        staticCollisions.push_back(collisionComponent);
    }
}

void TileCollisionAdapter::Render(HDC hdc)
{
    for (const auto& comp : staticCollisions)
        comp->Render(hdc);
}
