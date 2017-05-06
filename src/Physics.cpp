#include "Physics.h"

Physics::Physics() {}
void Physics::AddToPhysicsUpdate(Background* decoration) {
	// reserve space in 2d vector
	colliders.resize(decoration->height, vector<AABB*>(decoration->width, new AABB(0,0,0,0)));
	// add all AABBs
	for (int y = 0; y < decoration->height; y++) {
		for (int x = 0; x < decoration->width; x++) {
			if (decoration->GetTile(x, y)->isCollidable) {
				// create a rectangle with the same position and 
				AABB* aabb; aabb = new AABB(x * 64, y * 64, 64, 64);
				colliders[y][x]= aabb;
			}
		}
	}
}
void Physics::AddToPhysicsUpdate(Actor* actor) {
	actors.push_back(actor);
}
void Physics::DetectCollisions() {
	// for every actor, check the adjacent 9 squares
	// this may look like more work, but its actually more efficient
	vector<Actor*>::iterator actor_iter = actors.begin();
	for (actor_iter = actors.begin(); actor_iter != actors.end(); ++actor_iter) {
		int actor_x = (*actor_iter)->x / 64;
		int actor_y = (*actor_iter)->y / 64;
		for (int y = actor_y - 1; y < actor_y + 2; y++) {
			for (int x = actor_x - 1; x < actor_x + 2; x++) {
				if (IsCollidable(x, y)) {
					if (AABBIntersect((*actor_iter)->BoxCollider, colliders[y][x])) {
						// collision resolution
						int* oldPosition = (*actor_iter)->previous;
						int current[2] = { (*actor_iter)->x, (*actor_iter)->y };
						// move to where they wanted to go in the x
						(*actor_iter)->Move(current[0], oldPosition[1]);
						if (AABBIntersect((*actor_iter)->BoxCollider, colliders[y][x])) {
							// move them back
							(*actor_iter)->Move(oldPosition[0], oldPosition[1]);
							// move to wher they wanted to go in the y
							(*actor_iter)->Move(oldPosition[0], current[1]);
							// now test agains all boxes
							bool stillOverlap = 0;
							for (int y2 = actor_y - 1; y2 < actor_y + 2; y2++) {
								for (int x2 = actor_x - 1; x2 < actor_x + 2; x2++) {
									stillOverlap = AABBIntersect((*actor_iter)->BoxCollider, colliders[y][x]);
								}
							}
							if (stillOverlap) (*actor_iter)->Move(oldPosition[0], oldPosition[1]);
						}
					}
				}
			}
		}
	}
}

bool Physics::IsCollidable(int x, int y) {
	if (x < 0 || y < 0) return 0;
	if (y >= colliders.size() || x >= colliders[y].size()) return 0;
	return (colliders[y][x]->width > 0);
}