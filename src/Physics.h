#pragma once

#include <vector>
#include "Rendering.h"
using namespace std;

#ifndef PHYSICS_H
#define PHYSICS_H

/** This class determines what to do given actors, projectiles, and decorations */
class Physics {
public:
	Physics();
	void AddToPhysicsUpdate(Background* decoration);
	void AddToPhysicsUpdate(Actor* actor);
	// Physics loop
	void Update(float deltaTime); // update the requested movements (projectiles)
	void DetectCollisions(); // Check all actors and tiles for collisions
private:
	Background* decorations;
	vector<Actor*> actors;
};

#endif
