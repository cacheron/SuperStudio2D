#include "Rendering.h"
#include "DrawUtils.h"

// Tile code implementation
Tile::Tile() {
	x = 0.0; y = 0.0; img = 0;
	width = 0; height = 0;
	isCollidable = false;
}
Tile::Tile(float xPos, float yPos, int w, int h, int image, bool collision) {
	x = xPos; x = yPos; img = image;
	width = w;
	height = h;
	isCollidable = collision;
}

// Frame class implementation
Frame::Frame() {
	x = 0.0; y = 0.0;
	width = 0; height = 0;
	isCollidable = false;
	duration = 0.0;
}
Frame::Frame(float xPos, float yPos, int w, int h, int image, bool collision, float dur) {
	x = xPos; x = yPos; img = image;
	width = w;
	height = h;
	isCollidable = collision;
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
}
void Animation::SetAnimation(vector<Frame*>& newAnimation) {
	frameCount = newAnimation.size(); elapsedTime = 0.0;
	animation.clear();
	ReserveFrames();
	animation = newAnimation;
}
void Animation::DrawAnimation(float deltaTime, int xPos, int yPos, bool repeat) {
	if (!isFinished) {
		elapsedTime += deltaTime;
		if (elapsedTime >= animation[currentFrame]->duration) {
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
	return tileSet.at(level[x][y]);
}
void Background::SetLevel(vector< vector<int>>& newLevel) {
	level = newLevel;
}
void Background::Draw(int xPix, int yPix, int xTile, int yTile, int w, int h) {
	for (int xIndex = xTile; xIndex < xTile + w; ++xIndex) { // adjust width + height relative
		for (int yIndex = yTile; yIndex < yTile + h; ++yIndex) { // to the cam tile position
			Tile tile = *GetTile(yIndex, xIndex);
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
	// Draw Entities on the screen
	decoration->Draw(x, y, xTile, yTile, width, height);
	// Draw Sprites on the screen
}
void Camera::Move(float deltaTime, int direction[2]) {
	x += direction[0] * deltaTime * speed;
	y += direction[1] * deltaTime * speed;
	BoxCollider.Move(x, y); // update the box collider
	GetTileIndex(); // update tile index
}
void Camera::AddBackground(Background& level) {
	bg = &level;
}
void Camera::AddDecoration(Background& level) {
	decoration = &level;
}
void Camera::GetTileIndex() {
	xTile = x / 64;
	yTile = y / 64;
}