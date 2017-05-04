#pragma once
#include <vector>
#include "AABB.h"
using namespace std;

#ifndef RENDERING_H
#define RENDERING_H

/**
	The base class for the engine.
	A Tile has a position, a width/height,
	and image
*/
class Tile {
public:
	float x, y;
	int width, height, img;
	bool isCollidable;
	AABB* BoxCollider;
	Tile();
	Tile(float xPos, float yPos, int w, int h, int img, bool collision); // Create a tile with x and y pos, int width and height
};

/** A Frame class is a Tile with a duration */
class Frame : public Tile {
public:
	float duration;
	Frame();
	Frame(float xPos, float yPos, int w, int h, int img, bool collision, float dur);
};

/** An animation is a collection of frames */
class Animation {
public:
	bool isFinished, repeat;
	Animation();
	Animation(vector<Frame*>& frameVector, bool isFinished, bool repeat);
	void SetAnimation(Animation& newAnimation);
	void Draw(float deltaTime, int xPos, int yPos, bool repeat);
private:
	int currentFrame;
	int frameCount;
	float elapsedTime;
	vector<Frame*> animation;
	void ReserveFrames();
};

/** An Actor is a tile that can move and has animations */
class Actor : public Tile {
public:
	int input[2], health, speed;
	Actor();
	Actor(float xPos, float yPos, int w, int h, int hp, int spd, int animCount);
	void AddAnimation(Animation* animation);
	void SetInput(int in[2]);
	void Move(float deltaTime);
	void Update(float deltaTime); // update movement and animations
	void Draw(float deltaTime, int xPos, int yPos); // draw the current animation
private:
	Animation* currentAnimation;
	vector<Animation*> animations;
};
/** A background is a collection of Tiles, can draw them from a start and end index */
class Background {
public:
	int height, width; // level size
	Background();
	Background(int w, int h);
	void AddToTileSet(Tile* tile);
	Tile* GetTile(int x, int y);
	void SetLevel(vector< vector<int> >& newLevel);
	void Draw(int xPix, int yPix, int xTile, int yTile, int w, int h);
private:
	vector<Tile*> tileSet;
	vector< vector<int> > level;
	void ReserveLevel();
};

/** A camera is an object with position, width, and height. 
	It tells objects to draw themselves with an offset 
	according to its location */
class Camera {
public:
	int x, y;
	int width, height;
	int speed;
	Camera();
	Camera(int xPos, int yPos, int w, int h, int spd);
	void Draw(float deltaTime);
	void Move(float deltaTime, int direction[2]);
	void AddBackground(Background& level); // Adds the background to the camera
	void AddDecoration(Background& level); // Adds the decorations to the camera
	void AddActor(Actor* actor);
private:
	int xTile, yTile;
	Background* bg;
	Background* decoration;
	vector<Actor*> actors;
	AABB BoxCollider;
	void GetTileIndex();
};
#endif