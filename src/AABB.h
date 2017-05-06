#pragma once

#ifndef AABB_H
#define AABB_H

// an Axis Aligned Bounding Box Class for collisions
class AABB {
public:
	int x, y; // x and y position in pixels
	int width, height; // width and height in pixels!!
	AABB(); // default constructor
	AABB(int xPos, int yPos, int w, int h);
	// detect an intersection given a 
	bool Intersect(AABB* other);
	// update the position of the collider
	void Move(int xPos, int yPos);
};
// procedural function for AABB collisions
bool AABBIntersect(AABB* box1, AABB* box2);
int* AABBOverlap(AABB* box1, AABB* box2);
#endif