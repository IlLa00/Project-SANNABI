#pragma once
#include "TileMap.h"

class CollisionComponent;

class TileCollisionAdapter
{
public:
	void LoadFromTileMap(const vector<CollisionRect>& Collisions);
	void Render(HDC hdc);

private:
	vector<CollisionComponent*> staticCollisions;
};

