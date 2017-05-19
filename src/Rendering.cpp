#include "Rendering.h"
#include "DrawUtils.h"

using namespace std;

// Tile code implementation
Tile::Tile() {
	x = 0.0; y = 0.0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
}
Tile::Tile(int xPos, int yPos, int w, int h, int image, bool collision) {
	x = xPos; x = yPos; img = image;
	width = w;
	height = h;
	isCollidable = collision;
	if (isCollidable) {
		BoxCollider = new AABB(x, y, width, height);
	}
}

// Projectile class
Projectile::Projectile() {
	x = 0.0; y = 0.0; img = 0;
	width = 0; height = 0;
	isCollidable = true;
	speed = 0;
	collision = false;
	damage = 5;
}
Projectile::Projectile(int xPos, int yPos, int w, int h, int image, int spd, int dmg, bool coll) {
	x = xPos; x = yPos; img = image;
	width = w;
	height = h;
	isCollidable = coll;
	if (isCollidable) {
		BoxCollider = new AABB(x, y, width, height);
	}
	speed = spd;
	collision = false;
	damage = dmg;
}
void Projectile::Draw(int xPos, int yPos) {
	glDrawSprite(img, xPos, yPos, width, height);
}
void Projectile::Move(float deltaTime) {
	x += dir[0] * deltaTime * speed;
	y += dir[1] * deltaTime * speed;
	if (collision) { x = -64; y = -64; } // hide the proj
	BoxCollider->x = x; BoxCollider->y = y;
}

//Frame class implementation
Frame::Frame() {
	x = 0.0; y = 0.0;
	width = 0; height = 0;
	isCollidable = false;
	duration = 0.0;
}
Frame::Frame(int xPos, int yPos, int w, int h, int image, bool collision, float dur) {
	x = xPos; x = yPos; img = image;
	width = w;
	height = h;
	isCollidable = collision;
	if (isCollidable) {
		BoxCollider = new AABB(x, y, width, height);
	}
	duration = dur;
}

// Animation class implementation
Animation::Animation() {
	isFinished = true; repeat = false;
	currentFrame = 0; frameCount = 0; elapsedTime = 0.0;
	ReserveFrames();
}
Animation::Animation(vector<Frame*>& frameVector, bool finished, bool rpt) {
	frameCount = frameVector.size();
	ReserveFrames();
	animation = frameVector;
	isFinished = finished; repeat = rpt; elapsedTime = 0.0;
	currentFrame = 0;
}
void Animation::SetAnimation(Animation& newAnimation) {
	frameCount = newAnimation.frameCount; elapsedTime = 0.0;
	animation.clear();
	ReserveFrames();
	isFinished = newAnimation.isFinished; repeat = newAnimation.repeat;
	animation = newAnimation.animation;
}
void Animation::Draw(float deltaTime, int xPos, int yPos, bool repeat) {
	if (!isFinished) {
		elapsedTime += deltaTime;
		if (elapsedTime >= animation.at(currentFrame)->duration) {
			// switch the animation
			currentFrame++;	elapsedTime = 0.0;
			if (currentFrame >= frameCount) {
				if (repeat) { currentFrame = 0; }
				else { isFinished = true; currentFrame = frameCount-1; }
			}
		}
	}	
	// draw the current frame
	glDrawSprite(animation[currentFrame]->img, xPos, yPos, animation[currentFrame]->width, animation[currentFrame]->height);
}
void Animation::ReserveFrames() { animation.reserve(frameCount); }

// Actor implementation
Actor::Actor() {
	x = 0; y = 0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
	input[0] = 0; input[1] = 0; health = 0; speed = 0;
}
Actor::Actor(int xPos, int yPos, int w, int h, int hp, int spd, int animCount) {
	x = xPos; y = yPos; img = 0;
	width = w;
	height = h;
	isCollidable = true;
	BoxCollider = new AABB(x + (width / 16), y + (height / 16), (width / 4) * 3.5, (height / 4) * 3.5);
	health = hp; speed = spd;
	input[0] = 0; input[1] = 0;
	animations.reserve(animCount);
}
void Actor::AddAnimation(Animation * animation) {
	animations.push_back(animation);
	currentAnimation = animations[0];
}
void Actor::SetInput(int in[2]) {
	input[0] = in[0]; input[1] = in[1];
}
void Actor::Move(float deltaTime) {
	previous[0] = x; previous[1] = y;
	x += input[0] * deltaTime * speed;
	y += input[1] * deltaTime * speed;
	if (x < 0) x = 0; if (y < 0) y = 0;
	BoxCollider->x = x; BoxCollider->y = y;
}
void Actor::Move(int newX, int newY) {
	x = newX; y  = newY;
	BoxCollider->x = x; BoxCollider->y = y;
}
void Actor::Update(float deltaTime) {
	// for now, index 0 is idle, 1 is up, 2 is down, 3 is left, 4 is right
	// Up and down
	if (health < 0) {
		input[0] = 0; input[1] = 0;
		// set current animation
		currentAnimation = animations.at(4);
	} else {
		if (input[1] > 0) { currentAnimation = animations.at(1); }
		else if (input[1] < 0) { currentAnimation = animations.at(0); }
		// left and right
		if (input[0] > 0) { currentAnimation = animations.at(3); }
		else if (input[0] < 0) { currentAnimation = animations.at(2); }
	}
	// Move the player
	Move(deltaTime);
}
void Actor::Draw(float deltaTime, int xPos, int yPos) {
	currentAnimation->Draw(deltaTime, xPos, yPos, currentAnimation->repeat);
}
void Actor::TakeDamage(int dmg) {
	health -= dmg;
}

// Player Implementation
Player::Player() {
	x = 0; y = 0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
	input[0] = 0; input[1] = 0; health = 0; speed = 0;
	isPunching = false;
	punchPress = false;
	lookDir[0] = 0; lookDir[1] = 0;
	punch = new Projectile(-64, -64, 64, 64, 0, 0, 1, true);
}
Player::Player(int xPos, int yPos, int w, int h, int hp, int spd, int animCount) {
	x = xPos; y = yPos; img = 0;
	width = w;
	height = h;
	isCollidable = true;
	BoxCollider = new AABB(x + (width / 16), y + (height / 16), (width / 4) * 3.5, (height / 4) * 3.5);
	health = hp; speed = spd;
	input[0] = 0; input[1] = 0;
	animations.reserve(animCount);
	isPunching = false;
	punchPress = false;
	lookDir[0] = 0; lookDir[1] = 0;
	punch = new Projectile(-64, -64, 64, 64, 0, 0, 1, true);
}
void Player::Punch() {
	if (!punchPress && !isPunching) {
		punchPress = 1;
		// Move the projectile to the right position
		punch->x = lookDir[0] * 62 + x;
		punch->y = lookDir[1] * 62 + y;
		punch->BoxCollider->x = punch->x;
		punch->BoxCollider->y = punch->y;
		punch->dir[0] = 0; punch->dir[1] = 0;
		punch->collision = 0;
	}
}
void Player::Update(float deltaTime) {
	// for now, index 0 is up, 1 is down, 2 is left, 3 is right
	// 5 is dead
	// punches are 5 -> 8
	// Up and down
	if (health < 0) {
		input[0] = 0; input[1] = 0;
		// set current animation
		currentAnimation = animations.at(4);
	} else {
		if (punchPress) {
			punchPress = 0;
			isPunching = 1;
			// set the animation
			if (lookDir[1] > 0) { currentAnimation = animations.at(6); }
			else if (lookDir[1] < 0) { currentAnimation = animations.at(5); }
			// left and right
			if (lookDir[0] > 0) { currentAnimation = animations.at(8); }
			else if (lookDir[0] < 0) { currentAnimation = animations.at(7); }
		}
		if (isPunching && currentAnimation->isFinished) {
			currentAnimation->elapsedTime = 0.0f;
			currentAnimation->isFinished = 0;
			isPunching = 0;
			punch->x = -64; punch->y = -64;
			punch->BoxCollider->y = punch->y;
			punch->collision = 0;
		}
		if (!isPunching) {
			// up and down
			if (lookDir[1] > 0) { currentAnimation = animations.at(1); }
			else if (lookDir[1] < 0) { currentAnimation = animations.at(0); }
			// left and right
			if (lookDir[0] > 0) { currentAnimation = animations.at(3); }
			else if (lookDir[0] < 0) { currentAnimation = animations.at(2); }
		}
		if (lookDir[0] == 0 && lookDir[1] == 0) currentAnimation = animations.at(1);
	}
	// Move the player
	if (!isPunching) {
		Move(deltaTime);
	}
}
void Player::Move(float deltaTime) {
	previous[0] = x; previous[1] = y;
	if (input[0] ^ input[1]) {
		lookDir[0] = input[0];
		lookDir[1] = input[1];
	}
	x += input[0] * deltaTime * speed;
	y += input[1] * deltaTime * speed;
	if (x < 0) x = 0; if (y < 0) y = 0;
	BoxCollider->x = x; BoxCollider->y = y;
}

// Sentry Implementation
Sentry::Sentry() {
	x = 0; y = 0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
	input[0] = 0; input[1] = 0; health = 0; speed = 0;
	animations.reserve(0);
	status = "patrol";
	currentPoint[0] = x; currentPoint[1] = y;
	pathIndex = 0; bool reverse = false;
	range = 0;
	target[0] = 0; target[1] = 0;
	Behavior.CHASE = 0.33;
	Behavior.RUN = 0.33;
	Behavior.SHOOT = 0.33;
	decision = -1.0;
	peanut = new Projectile(-64, -64, 64, 64, 0, 180, 5, true);
	fired = false;
}
Sentry::Sentry(int xPos, int yPos, int w, int h, int hp, int spd, int rng, int animCount) {
	x = xPos; y = yPos; img = 0;
	width = w;
	height = h;
	isCollidable = true;
	BoxCollider = new AABB(x + (width / 16), y + (height / 16), (width / 4) * 3.5, (height / 4) * 3.5);
	health = hp; speed = spd;
	input[0] = 0; input[1] = 0;
	animations.reserve(animCount);
	status = "patrol";
	currentPoint[0] = x; currentPoint[1] = y;
	pathIndex = 0; bool reverse = false;
	range = rng;
	target[0]=0; target[1] = 0;
	Behavior.CHASE = 0.33;
	Behavior.RUN = 0.33;
	Behavior.SHOOT = 0.33;
	decision = -1.0;
	peanut = new Projectile(-64, -64, 64, 64, 0, 180, 5, true);
	fired = false;
}
void Sentry::SetPath(vector<int*> newPath) {
	path = newPath;
	currentPoint[0] = path[0][0];
	currentPoint[1] = path[0][1];
}
void Sentry::SetTarget(int x, int y) {
	target[0] = x; target[1] = y;
}
void Sentry::SetBehavior(float chase, float run, float shoot) {
	Behavior.CHASE = chase;
	Behavior.RUN = run;
	Behavior.SHOOT = shoot;
}
void Sentry::SetDecision() {
	if (decision == -1.0) {
		decision = (float)rand() / (float)RAND_MAX;
	}
	if (decision > 0 && decision <= Behavior.CHASE) {
		status = "chase";
	}
	if (decision > Behavior.CHASE && decision <= Behavior.CHASE + Behavior.RUN) {
		status = "run";
	}
	if (decision > Behavior.CHASE + Behavior.RUN && decision <= 1.0) {
		status = "shoot";
	}
}
void Sentry::SetInput() {
	int xDir = currentPoint[0] * 64 - x;
	int yDir = currentPoint[1] * 64 - y;
	input[0] = 0; input[1] = 0;
	if (yDir < 0)	input[1] = -1; // move up
	if (yDir > 0) input[1] = 1; // move down
	if (xDir < 0) input[0] = -1; // move left
	if (xDir > 0) input[0] = 1; // move right
	if (status == "run") {
		input[0] *= -1;
		input[1] *= -1;
	}
	if (status == "shoot") {
		Shoot(xDir, yDir);
	}
}
void Sentry::Shoot(int xDir, int yDir) {
	// now we need to fire a peanut at the player
	if (!fired) { // havent fired yet, set the peanut to correct position
		peanut->x = x; peanut->y = y;
		peanut->BoxCollider->x = x; peanut->BoxCollider->x = x;
		float total = (abs(xDir) + abs(yDir));
		peanut->dir[0] = xDir / total;
		peanut->dir[1] = yDir / total;
		peanut->collision = 0;
		fired = 1;
	}
	else {
		if (peanut->collision) fired = 0;
	}
	input[0] = 0; input[1] = 0;
}
void Sentry::SetIMG(int img) {
	peanut->img = img;
}
void Sentry::Update(float deltaTime) {
	// Here, the sentry both updates the animation and
	// Decides on an action to take
	if (health > 0) {
		UpdateDecision(deltaTime);
		UpdateAnimation(deltaTime);
	} else {
		// he ded
		BoxCollider->height = 0; BoxCollider->width = 0;
		BoxCollider->x = -128; BoxCollider->y = -128;
		// play ded animation
		currentAnimation = animations.at(4);
	}
}
bool Sentry::AtDestination() {
	return (x / 64 == currentPoint[0] && y / 64 == currentPoint[1]);
}
bool Sentry::DetectPlayer() {
	int distX = target[0] - x; int distY= target[1] - y;
	float distance = (distX * distX) + (distY * distY);
	if (distance <= range * range) {
		return 1;
	}
	return 0;
}
void Sentry::UpdateDecision(float deltaTime) {
	// for now, sentry can only walk back and forth between the points
	// Detect if we can see the player:
	if (DetectPlayer()) {
		// Here the AI is chasing/running from/shooting the player
		currentPoint[0] = target[0] / 64;
		currentPoint[1] = target[1] / 64;
		// Then he must decide which behavior he wishes to do
		SetDecision();
	}
	else { // We dont know where the player is, so keep patrolling
		if (status != "patrol") {
			// go back to original point
			pathIndex = 0; decision = -1.0;
			currentPoint[0] = path[pathIndex][0];
			currentPoint[1] = path[pathIndex][1];
			status = "patrol";
		}
		else {
			if (AtDestination()) {
				// time to move to the next point
				if (!reverse) {
					pathIndex++;
					if (pathIndex >= path.size()) {
						reverse = true;
						pathIndex -= 2;
					}
				}
				else {
					pathIndex--;
					if (pathIndex < 0) {
						reverse = false;
						pathIndex += 2;
					}
				}
				currentPoint[0] = path[pathIndex][0];
				currentPoint[1] = path[pathIndex][1];
			}
		}
	}
	SetInput();
	// Move the player
	Move(deltaTime);
}
void Sentry::UpdateAnimation(float deltaTime) {
	// Up and down
	if (input[1] > 0) { currentAnimation = animations.at(1); }
	else if (input[1] < 0) { currentAnimation = animations.at(0); }
	// left and right
	if (input[0] > 0) { currentAnimation = animations.at(3); }
	else if (input[0] < 0) { currentAnimation = animations.at(2); }
}

// Background class implementation
Background::Background() {
	width = 0; height = 0;
	ReserveLevel();
}
Background::Background(int w, int h) {
	width = w; height = h;
	ReserveLevel();
}
void Background::AddToTileSet(Tile* tile) {
	tileSet.push_back(tile);
}
Tile* Background::GetTile(int x, int y) {
	if ( (x >= width || x <= -1) || 
		 (y >= height || y <= -1) ) {
		return tileSet[0];
	}
	return tileSet.at(level[y][x]);
}
void Background::SetLevel(vector< vector<int>>& newLevel) {
	level = newLevel;
}
void Background::Draw(int xPix, int yPix, int xTile, int yTile, int w, int h) {
	for (int yIndex = yTile; yIndex < yTile + h; ++yIndex) { // adjust width + height relative
		for (int xIndex = xTile; xIndex < xTile + w; ++xIndex) { // to the cam tile position
			Tile tile = *GetTile(xIndex, yIndex);
			int xPos = (xIndex * tile.width) - xPix; // adjust drawing relative to origin
			int yPos = (yIndex * tile.height) - yPix;
			glDrawSprite(tile.img, xPos, yPos, tile.height, tile.width);
		}
	}
}
void Background::ReserveLevel() {
	level.resize(width, vector<int>(height, 0));
}

// Camera class implementation
Camera::Camera() {
	x = 0; y = 0; width = 0; height = 0; speed = 0;
	xTile = 0; yTile = 0;
	BoxCollider.x = 0; BoxCollider.y = 0;
	BoxCollider.width = 0; BoxCollider.height = 0;
}
Camera::Camera(int xPos, int yPos, int w, int h, int spd) {
	x = xPos; y = yPos; width = w; height = h; speed = spd;
	BoxCollider.x = x; BoxCollider.y = y;
	BoxCollider.width = width; BoxCollider.height = height;
	GetTileIndex();
}
void Camera::Draw(float deltaTime) {
	// Draw the background
	bg->Draw(x, y, xTile, yTile, width, height);
	// Draw Sprites on the screen
	vector<Actor*>::iterator actor_iter = actors.begin();
	for (actor_iter = actors.begin(); actor_iter != actors.end(); ++actor_iter) {
		(*actor_iter)->Draw(deltaTime, (*actor_iter)->x - x, (*actor_iter)->y - y);
	}
	// Draw Entities on the screen
	decoration->Draw(x, y, xTile, yTile, width, height);
	// Draw projectiles
	vector<Projectile*>::iterator proj_iter = projectiles.begin();
	for (proj_iter = projectiles.begin(); proj_iter != projectiles.end(); ++proj_iter) {
		(*proj_iter)->Draw((*proj_iter)->x - x, (*proj_iter)->y - y);
	}
}
void Camera::Move(float deltaTime, int direction[2]) {
	x += direction[0] * deltaTime * speed;
	y += direction[1] * deltaTime * speed;
	if (x < 0) x = 0; if (y < 0) y = 0;
	if (y / 64 >= 30) y = 30 * 64;
	BoxCollider.Move(x, y); // update the box collider
	GetTileIndex(); // update tile index
}
void Camera::AddBackground(Background& level) {
	bg = &level;
}
void Camera::AddDecoration(Background& level) {
	decoration = &level;
}
void Camera::AddActor(Actor* actor) {
	actors.push_back(actor);
}
void Camera::AddProjectile(Projectile* proj) {
	projectiles.push_back(proj);
}
void Camera::GetTileIndex() {
	xTile = x / 64;
	yTile = y / 64;
}