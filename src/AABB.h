#pragma once

#ifndef AABB_H
#define AABB_H

// an Axis Aligned Bounding Box Class for collisions
class AABB {
public:
	float x, y; // x and y position in pixels
	int width, height; // width and height in pixels!!
	AABB(); // default constructor
	AABB(int xPos, int yPos, int w, int h);
	// detect an intersection given a 
	bool Intersect(AABB* other);
	// update the position of the collider
	void Move(int xPos, int yPos);
};
// procedural function for AABB collisions
bool AABBIntersect(int x, int y, int w, int h, int x2, int y2, int w2, int h2);

#endif