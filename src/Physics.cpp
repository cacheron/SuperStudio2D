#include "Physics.h"

Physics::Physics() {}
void Physics::AddToPhysicsUpdate(Background* decoration) {
	// reserve space in 2d vector
	colliders.resize(decoration->height, vector<AABB*>(decoration->width, new AABB(0, 0, 0, 0)));
	// add all AABBs
	for (int y = 0; y < decoration->height; y++) {
		for (int x = 0; x < decoration->width; x++) {
			if (decoration->GetTile(x, y)->isCollidable) {
				// create a rectangle with the same position and 
				AABB* aabb; aabb = new AABB(x * 64, y * 64, 64, 64);
				colliders[y][x] = aabb;
			}
		}
	}
}
void Physics::AddToPhysicsUpdate(Actor* actor) {
	actors.push_back(actor);
}
void Physics::AddToPhysicsUpdate(Projectile* proj) {
	projectiles.push_back(proj);
}
bool Physics::IsCollidable(int x, int y) {
	if (x < 0 || y < 0) return 0;
	if (y >= colliders.size() || x >= colliders[y].size()) return 0;
	return (colliders[y][x]->width > 0);
}
void Physics::DetectCollisions() {
	// for every actor, check the adjacent 9 squares
	// this may look like more work, but its actually more efficient
	vector<Actor*>::iterator actor_iter = actors.begin();
	for (actor_iter = actors.begin(); actor_iter != actors.end(); ++actor_iter) {
		// this actor is an enemy
		int actor_x = (*actor_iter)->x / 64;
		int actor_y = (*actor_iter)->y / 64;
		int max_x = actor_x + 2;
		int max_y = actor_y + 2;
		int y = actor_y - 1;
		int x = actor_x - 1;
		for (int yi = y; yi < max_y; yi++) {
			for (int xi = x; xi < max_x; xi++) {
				if (IsCollidable(xi, yi)) {
					if (AABBIntersect((*actor_iter)->BoxCollider, colliders[yi][xi])) {
						// collision resolution
						int* oldPosition = (*actor_iter)->previous;
						int current[2] = { (*actor_iter)->x, (*actor_iter)->y };
						// move to where they wanted to go in the x
						(*actor_iter)->Move(current[0], oldPosition[1]);
						if (AABBIntersect((*actor_iter)->BoxCollider, colliders[yi][xi])) {
							// This prevents movement through x direction
							// move them back
							(*actor_iter)->Move(oldPosition[0], oldPosition[1]);
							// move to wher they wanted to go in the y
							(*actor_iter)->Move(oldPosition[0], current[1]);
						}
						if (IsCollidable(xi, yi + 1) && (AABBIntersect((*actor_iter)->BoxCollider, colliders[yi + 1][xi]))) {
							(*actor_iter)->Move(oldPosition[0], current[1]);
							// moving up along a wall
						}
						
					}
				}
			}
		}
		if (actor_iter != actors.begin()) {
			for (int i = 0; i < actors.size(); i++) {
				if ((*actor_iter) != actors[i]) {
					if (AABBIntersect((*actor_iter)->BoxCollider, actors[i]->BoxCollider)) {
						actors[i]->TakeDamage(1);
						// collision resolution
						int* oldPosition = (*actor_iter)->previous;
						int current[2] = { (*actor_iter)->x, (*actor_iter)->y };
						// move to where they wanted to go in the x
						(*actor_iter)->Move(current[0], oldPosition[1]);
						if (AABBIntersect((*actor_iter)->BoxCollider, actors[i]->BoxCollider)) {
							// move them back
							(*actor_iter)->Move(oldPosition[0], oldPosition[1]);
							// move to wher they wanted to go in the y
							(*actor_iter)->Move(oldPosition[0], current[1]);
						}
					}
				}
			}
		}
	}
	
	// projectiles
	vector<Projectile*>::iterator proj_iter = projectiles.begin();
	for (proj_iter = projectiles.begin(); proj_iter != projectiles.end(); ++proj_iter) {
		int proj_x = (*proj_iter)->x / 64;
		int proj_y = (*proj_iter)->y / 64;
		for (int y = proj_y - 1; y < proj_y + 2; y++) {
			for (int x = proj_x - 1; x < proj_x + 2; x++) {
				if (IsCollidable(x, y)) {
					if (AABBIntersect((*proj_iter)->BoxCollider, colliders[y][x])) {
						// collision resolution
						(*proj_iter)->collision = 1;
					}
				}
			}
		}
		if (proj_iter != projectiles.begin() && proj_iter != projectiles.begin()+1) {
			// These are enemy projectiles
			// see if we hit an actor
			if (AABBIntersect((*proj_iter)->BoxCollider, actors[0]->BoxCollider)) {
				// do damage and set proj coll to true
				(*proj_iter)->collision = 1;
				actors[0]->TakeDamage((*proj_iter)->damage);
			}
		}
		else if (proj_iter == projectiles.begin()) {
			// This is the player's punch
			for (int i = 1; i < actors.size(); i++) { // collision with enemy
				if (AABBIntersect((*proj_iter)->BoxCollider, actors[i]->BoxCollider)) {
					// do damage and set proj coll to true
					(*proj_iter)->collision = 1;
					actors[i]->TakeDamage((*proj_iter)->damage);
				}
			}
			for (int i = 2; i < projectiles.size(); i++) { // collision with other projectiles
				if (AABBIntersect((*proj_iter)->BoxCollider, projectiles[i]->BoxCollider)) {
					(*proj_iter)->collision = 1;
					projectiles[i]->collision = 1;
				}
			}
			if (AABBIntersect((*proj_iter)->BoxCollider, actors[0]->BoxCollider)) { (*proj_iter)->collision = 1; }
		} else {
			if (AABBIntersect((*proj_iter)->BoxCollider, actors[0]->BoxCollider)) {
				// Player got the key
				((Player*)actors[0])->hasKey = true;
				(*proj_iter)->collision = 1;

			}
		}
	}
}
void Physics::Update(float deltaTime) {
	// Update the physics of moving objects that dont update themselves
	vector<Projectile*>::iterator proj_iter = projectiles.begin();
	for (proj_iter = projectiles.begin(); proj_iter != projectiles.end(); ++proj_iter) {
		(*proj_iter)->Move(deltaTime);
	}
}
void Physics::ResetComponents() {
	colliders.clear();
	actors.clear();
	for (int i = 0; i < projectiles.size(); i++) {
		projectiles[i]->collision = 1;
	}
	projectiles.clear();
}