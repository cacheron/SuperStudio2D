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
	int x, y;
	int width, height, img;
	bool isCollidable;
	AABB* BoxCollider;
	Tile();
	Tile(int xPos, int yPos, int w, int h, int img, bool collision); // Create a tile with x and y pos, int width and height
};

class Projectile : public Tile {
public:
	float dir[2]; float speed; 
	int damage;
	bool collision;
	Projectile();
	Projectile(int xPos, int yPos, int w, int h, int img, int spd, int dmg, bool collision);
	void Draw(int x, int y);
	void Move(float deltaTime);
	void Move(int x, int y);
};

/** A Frame class is a Tile with a duration */
class Frame : public Tile {
public:
	float duration;
	Frame();
	Frame(int xPos, int yPos, int w, int h, int img, bool collision, float dur);
};

/** An animation is a collection of frames */
class Animation {
public:
	bool isFinished, repeat;
	Animation();
	Animation(vector<Frame*>& frameVector, bool isFinished, bool repeat);
	void SetAnimation(Animation& newAnimation);
	void Draw(float deltaTime, int xPos, int yPos, bool repeat);
	float elapsedTime;
private:
	int currentFrame;
	int frameCount;
	vector<Frame*> animation;
	void ReserveFrames();
};

/** An Actor is a tile that can move and has animations */
class Actor : public Tile {
public:
	int input[2], health, speed;
	int previous[2];
	Actor();
	Actor(int xPos, int yPos, int w, int h, int hp, int spd, int animCount);
	void AddAnimation(Animation* animation);
	void SetInput(int in[2]);
	void Move(float deltaTime); // move using input and delta time
	void Move(int addX, int addY); // move by adding to the x and y pos
	void Update(float deltaTime); // update movement and animations
	void Draw(float deltaTime, int xPos, int yPos); // draw the current animation
	void TakeDamage(int dmg); // take damage to health
protected:
	Animation* currentAnimation;
	vector<Animation*> animations;
};

class Player : public Actor {
public:
	bool hasKey;
	Player();
	Player(int xPos, int yPos, int w, int h, int hp, int spd, int animCount);
	Projectile* punch;
	void Punch();
	void Update(float deltaTime);
private:
	int lookDir[2];
	void Move(float deltaTime);
	bool isPunching, punchPress;
};

class Sentry : public Actor {
public:
	int range;
	Projectile* peanut;
	char* status;
	Sentry();
	Sentry(int xPos, int yPos, int w, int h, int hp, int spd, int rng, int animCount);
	void Update(float deltaTime); // Behaves like an actor with a script controlling its actions
	void SetPath(vector<int*> newPath);
	void SetTarget(int x, int y);
	void SetBehavior(float chase, float run, float shoot);
	void SetIMG(int img);
	void CopyValues(Sentry* copy); // Copy all values from one sentry into this sentry
private:
	// pathing vairables
	int currentPoint[2]; int target[2];
	int pathIndex; bool reverse;
	vector<int*> path;
	// behavior vars
	struct { float CHASE, RUN, SHOOT; } Behavior; float decision;
	// shooting vars
	bool fired;
	bool AtDestination();
	bool DetectPlayer();
	void SetInput();
	void SetDecision();
	void UpdateDecision(float deltaTime);
	void UpdateAnimation(float deltaTime);
	void Shoot(int xDir, int yDir);
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
	void SetTileSet(vector<Tile*> tiles);
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
	void Move(int newX, int newY);
	void SetBackground(Background* level); // Adds the background to the camera
	void SetDecoration(Background* level); // Adds the decorations to the camera
	void AddActor(Actor* actor);
	void AddProjectile(Projectile* proj);
	void ClearScreen();
private:
	int xTile, yTile;
	Background* bg;
	Background* decoration;
	vector<Actor*> actors;
	vector<Projectile*> projectiles;
	AABB BoxCollider;
	void GetTileIndex();
};
#endif