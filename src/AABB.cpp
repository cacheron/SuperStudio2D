#include "AABB.h"

// Axis Aligned Bounding Box implementation
AABB::AABB() { x = 0; y = 0; width = 0; height = 0; }
AABB::AABB(int xPos, int yPos, int w, int h) {
	x = xPos; y = yPos; width = w; height = h;
}
// AABB intersect with instances of AABB
bool AABB::Intersect(AABB* other) {
	// Note that something here messes up when
	// colliding from below
	if (y >= other->y + other->height)	return false;
	if (y + height <= other->y)			return false;
	if (x >= other->x + other->width)	return false;
	if (x + width <= other->x)			return false;
	return true;
}
void AABB::Move(int xPos, int yPos) {
	x = xPos; y = yPos;
}
// function for AABB intersect without the classes
bool AABBIntersect(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
	if (y >= y2 + h2)	return false;
	if (y + h <= y2)	return false;
	if (x >= x2 + w2)	return false;
	if (x + w <= x2)	return false;
	return true;
}