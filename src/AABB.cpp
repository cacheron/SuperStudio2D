#include "AABB.h"
#include <iostream>

// Axis Aligned Bounding Box implementation
AABB::AABB() { x = 0; y = 0; width = 0; height = 0; }
AABB::AABB(int xPos, int yPos, int w, int h) {
	x = xPos; y = yPos; width = w; height = h;
}
// AABB intersect with instances of AABB
bool AABB::Intersect(AABB* other) {
	// Note that something here messes up when
	// colliding from below
	if(x > other->x + other->width) {
		return false;
	}
	//box 1 to the left
	if(x + width < other->x) {
		return false;
	}
	//box1 is below
	if(y > other->y + other->height) {
		return false;
	}
	//box 1 is above
	if(y + height < other->y) {
		return false;
	}
	return true;
}
void AABB::Move(int xPos, int yPos) {
	x = xPos; y = yPos;
}
// function for AABB intersect without the classes
bool AABBIntersect(AABB* box1, AABB* box2) {
	//box1 to the right
	if (box1->x >= box2->x + box2->width) return 0;
	if (box1->x + box1->width <= box2->x) return 0;
	if (box1->y - 1 >= box2->y + box2->height) return 0;
	if (box1->y + box1->height + 1 <= box2->y) return 0;
	return 1;
}
int* AABBOverlap(AABB* box1, AABB* box2) {
	// how much should box1 be moved to get out of box2
	static int overlap[2] = {0,0};
	
	if (box2->x > box1->x + box1->width / 2 && box2->x < box1->x + box2->width) {
		//overlap is on the right side of the box.
		overlap[0] = (box1->x + box1->width) - box2->x + 1; // positive is right side
	}
	if (box2->x + box2->width > box1->x && box2->x + box2->width < box1->x + box2->width / 2) {
		//overlap is on the left side of the box.
		overlap[0] = -( (box2->x + box2->width) - box1->x + 1); // negative is left side
	}
	if (box2->y > box1->y + box1->height / 2 && box2->y < box1->y + box2->height) {
		//overlap is on the right side of the box.
		overlap[1] = (box1->y + box1->height) - box2->y + 1; // positive is right side
	}
	if (box2->y + box2->height > box1->y && box2->y + box2->height < box1->y + box2->height / 2) {
		//overlap is on the left side of the box.
		overlap[1] = -((box2->y + box2->height) - box1->y + 1); // negative is left side
	}

	return overlap;
}