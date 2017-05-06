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

// Frame class implementation
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
	// No diagonal input support!
	// Up and down
	if (input[1] > 0) { currentAnimation = animations.at(1); }
	else if (input[1] < 0) { currentAnimation = animations.at(0); }
	// left and right
	if (input[0] > 0) { currentAnimation = animations.at(3); }
	else if (input[0] < 0) { currentAnimation = animations.at(2); }
	// Move the player
	Move(deltaTime);
}
void Actor::Draw(float deltaTime, int xPos, int yPos) {
	if (input[0] == 0 && input[1] == 0) {
		currentAnimation = animations.at(1);
		currentAnimation->Draw(0.0, xPos, yPos, true);
	}
	else {
		currentAnimation->Draw(deltaTime, xPos, yPos, true);
	}
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
}
void Camera::Move(float deltaTime, int direction[2]) {
	x += direction[0] * deltaTime * speed;
	y += direction[1] * deltaTime * speed;
	if (x < 0) x = 0; if (y < 0) y = 0;
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
void Camera::GetTileIndex() {
	xTile = x / 64;
	yTile = y / 64;
}