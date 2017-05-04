#include "Physics.h"

Physics::Physics() {}
void Physics::AddToPhysicsUpdate(Background* decoration) {
	decorations = decoration;
}
void Physics::AddToPhysicsUpdate(Actor* actor) {
	actors.push_back(actor);
}
void Physics::DetectCollisions() {

}