#include "Level.h"

// Level Implementation

Level::Level() {
	gate_key = new Projectile(0, 0, 64, 64, 0, 0, 0, 1);
	gate = new Projectile(0, 0, 64, 64, 0, 0, 0, 1);
	gate_key_x = 0; gate_key_y = 0;
	gate_x = 0; gate_y = 0;
}
Level::Level(Background* bg, Background* deco, vector<Sentry*> enem) {
	background = bg; decorations = deco; enemies = enem;
	gate_key = new Projectile(0, 0, 64, 64, 0, 0, 0, 1);
	gate = new Projectile(0, 0, 64, 64, 0, 0, 0, 1);
	gate_key_x = 0; gate_key_y = 0;
	gate_x = 0; gate_y = 0;
}
void Level::RemoveAllEnemies() { enemies.clear(); currentEnemies.clear(); }
void Level::Reset() {
	currentEnemies.clear();
	if (!enemies.empty()) {
		for (int i = 0; i < enemies.size(); i++) {
			currentEnemies.push_back(new Sentry(*enemies[i]));
			currentEnemies[i]->BoxCollider->x = currentEnemies[i]->x;
			currentEnemies[i]->BoxCollider->y = currentEnemies[i]->y;
		}
	}
	// reset gate and key
	gate->x = -64; gate->y = -64;
	gate_key->x = gate_key_x; gate_key->y = gate_key_y;
}
void Level::SetCurrentEnemies() {
}
bool Level::IsGateOpen() {	return ((gate_key->x == -64) && (gate_key->y == -64)); }
void Level::OpenGate() {
	// move the gate projectile to the correct spot
	gate->Move(gate_x, gate_y);
}