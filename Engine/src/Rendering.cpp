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
	return tileSet.at(level[x][y]);
}
void Background::SetLevel(vector< vector<int>>& newLevel) {
	level = newLevel;
}
void Background::DrawBackground(int xPix, int yPix, int xTile, int yTile, int w, int h) {
	for (int xIndex = xTile; xIndex < xTile + w; ++xIndex) { // adjust width + height relative
		for (int yIndex = yTile; yIndex < yTile + h; ++yIndex) { // to the cam tile position
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