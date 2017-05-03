#pragma once
#include <vector>

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
	Tile();
	Tile(float xPos, float yPos, int w, int h, int img, bool collision); // Create a tile with x and y pos, int width and height
};

/** A Frame class is a Tile with a duration */
/** An animation is a collectio of frames */
/** A background is a collection of AABBTiles, can draw them from a start and end index */
class Background {
public:
	int height, width; // level size
	Background();
	Background(int w, int h);
	void AddToTileSet(Tile* tile);
	Tile* GetTile(int x, int y);
	void SetLevel(vector< vector<int> >& newLevel);
	void DrawBackground(int xPix, int yPix, int xTile, int yTile, int w, int h);
private:
	vector<Tile*> tileSet;
	vector< vector<int> > level;
	void ReserveLevel();
};


#endif