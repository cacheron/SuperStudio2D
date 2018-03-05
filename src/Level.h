#pragma once
#include <vector>
#include "Rendering.h"

#ifndef LEVEL_H
#define LEVEL_H
/*
A level class should hold all the necessary information for a level in the game
it has a background, decorations (collidables and entities), enemies, and
the ability to reset the level values and 
remove everything from the level when it is completed.
*/
class Level {
public:
	int gate_x = 0, gate_y = 0;
	int gate_key_x = 0, gate_key_y = 0;
	Projectile *gate_key;
	Projectile *gate;
	Level();
	Level(Background* bg, Background* deco, vector<Sentry*> enems);
	Background* background;
	Background* decorations;
	vector<Sentry*> enemies;
	vector<Sentry*> currentEnemies;
	bool IsGateOpen();
	void OpenGate(); // Move the gate (stairs) to a spot on the level
	void RemoveAllEnemies(); // remove all enemy objects from the vector
	void Reset(); // Reset enemies back to original positions with full health
	void SetCurrentEnemies();
};

#endif