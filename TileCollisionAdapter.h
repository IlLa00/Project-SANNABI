#pragma once

class CollisionComponent;

class TileCollisionAdapter
{
public:
	void LoadFromTileMap(const vector<RECT>& Collisions);
	void Render(HDC hdc);

private:
	vector<CollisionComponent*> staticCollisions;
};

