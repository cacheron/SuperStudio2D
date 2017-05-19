#include "Actors.h"

// Actor implementation
Actor::Actor() {
	x = 0.0; y = 0.0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
	input[0] = 0; input[1] = 0; health = 0; speed = 0;
}
Actor::Actor(float xPos, float yPos, int w, int h, int hp, int spd) {
	x = xPos; x = yPos; img = 0;
	width = w;
	height = h;
	isCollidable = true;
	BoxCollider = new AABB(x, y, width, health);
	health = hp; speed = spd;
	input[0] = 0; input[1] = 0;
}
void Actor::SetInput(int in[2]) {
	input[0] = in[0]; input[1] = in[1];
}
void Actor::Move(float deltaTime) {
	x += input[0] * deltaTime * speed;
	y += input[1] * deltaTime * speed;
}
void Actor::Update(float deltaTime) {
	// for now, index 0 is idle, 1 is up, 2 is down, 3 is left, 4 is right
	// No diagonal input support!
	// left and right have less priority
	if (input[0] > 0) { currentAnimation = animations[3]; }
	else if (input[0] < 0) { currentAnimation = animations[4]; }
	// Up and down should have priority
	if (input[1] > 0) { currentAnimation = animations[1]; }
	else if (input[1] < 0) { currentAnimation = animations[2];  }

	// Move the player
	Move(deltaTime);
}
void Actor::Draw(float deltaTime, int xPos, int yPos) {
	if (input[0] == 0 && input[1] == 0) {
		currentAnimation->Draw(0.0, xPos, yPos, true);
	} else {
		currentAnimation->Draw(deltaTime, xPos, yPos, true);
	}
}