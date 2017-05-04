#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "DrawUtils.h"
#include "Rendering.h"

// Set this to true to make the game loop exit.
char shouldExit = 0;

// The previous frame's keyboard state.
unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};

// The current frame's keyboard state.
const unsigned char* kbState = NULL;

// Position of the sprite.
float spritePos[2] = {10, 10};

// Texture for the sprite.
GLuint spriteTex;

// Size of the sprite.
int spriteSize[2];
int camPos[2];

int main(void)
{
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL. ErrorCode=%s\n", SDL_GetError());
        return 1;
    }

    // Create the window and OpenGL context.
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* window = SDL_CreateWindow(
            "SuperStudio2D",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600,
            SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "Could not create window. ErrorCode=%s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GL_CreateContext(window);
    // Make sure we have a recent version of OpenGL.
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Could not initialize glew. ErrorCode=%s\n", glewGetErrorString(glewError));
        SDL_Quit();
        return 1;
    }
    if (GLEW_VERSION_2_0) {
        fprintf(stderr, "OpenGL 2.0 or greater supported: Version=%s\n",
                 glGetString(GL_VERSION));
    } else {
        fprintf(stderr, "OpenGL max supported version is too low.\n");
        SDL_Quit();
        return 1;
    }

    // Setup OpenGL state.
    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, 800, 600, 0, 0, 100);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Game initialization goes here.
    spriteTex = glTexImageTGAFile("img/spr_player_front.tga", &spriteSize[0], &spriteSize[1]);
	
	// Calculation of deltaTime
	Uint64 NOW = SDL_GetTicks();
	Uint64 LAST = 0;
	Uint64 elapsedTime = 0;
	float deltaTime, seconds = 0;

	// Create the background
	int tex_w = 0; int tex_h = 0;
	int tex_broke = glTexImageTGAFile("img/tex_broke.tga", &tex_w, &tex_h);
	Tile* broke = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_broke, false);

	int tex_grass = glTexImageTGAFile("img/tex_grass.tga", &tex_w, &tex_h);
	Tile* grass = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_grass, false);

	int tex_carpet = glTexImageTGAFile("img/tex_carpet.tga", &tex_w, &tex_h);
	Tile* carpet = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_carpet, false);

	int level[12][12] = {	{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 }
						};
	// convert level to 2d vector
	vector<vector<int>> vectorLevel;
	for (int i = 0; i < 40; ++i) {
		vectorLevel.push_back(vector<int>(begin(level[i]), end(level[i])));
	}
	Background* bg = new Background(12, 12);
	bg->SetLevel(vectorLevel);
	bg->AddToTileSet(broke);
	bg->AddToTileSet(grass);
	bg->AddToTileSet(carpet);

	// Create the decoration background
	int spr_transparent = glTexImageTGAFile("img/spr_transparent.tga", &tex_w, &tex_h);
	Tile* transparent = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_transparent, false);

	int spr_window_left = glTexImageTGAFile("img/spr_window_left.tga", &tex_w, &tex_h);
	Tile* window_left = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_window_left, true);

	int spr_window_right = glTexImageTGAFile("img/spr_window_right.tga", &tex_w, &tex_h);
	Tile* window_right = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_window_right, true);

	int spr_chair = glTexImageTGAFile("img/spr_chair.tga", &tex_w, &tex_h);
	Tile* chair = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_chair, true);

	int spr_case = glTexImageTGAFile("img/spr_case.tga", &tex_w, &tex_h);
	Tile* red_case = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_case, true);

	int decorations[12][12] = { { 0, 1, 0, 0, 0, 0, 0, 4, 4, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 0, 0, 4, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 3, 3, 3, 2, 0 }
	};
	vector<vector<int>> vectordeco;
	for (int i = 0; i < 40; ++i) {
		vectordeco.push_back(vector<int>(begin(decorations[i]), end(decorations[i])));
	}
	Background* decoration = new Background(12, 12);
	decoration->SetLevel(vectordeco);
	decoration->AddToTileSet(transparent);
	decoration->AddToTileSet(window_left);
	decoration->AddToTileSet(window_right);
	decoration->AddToTileSet(chair);
	decoration->AddToTileSet(red_case);

	Camera* camera = new Camera(0, 0, 14, 11, 300);
	camera->AddBackground(*bg);
	camera->AddDecoration(*decoration);
    // The game loop.
	kbState = SDL_GetKeyboardState(NULL);
	while (!shouldExit) {
		NOW = SDL_GetTicks();
		deltaTime = (NOW - LAST) / 1000.0;
		LAST = NOW;
		seconds += deltaTime;
		if (static_cast<int>(seconds) == 1) {
			elapsedTime += 1; seconds = 0.0;
			printf("%d\n", elapsedTime);
		}
		
		if (elapsedTime )
        assert(glGetError() == GL_NO_ERROR);
        memcpy(kbPrevState, kbState, sizeof(kbPrevState));
        
        // Handle OS message pump.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    shouldExit = 1;
            }
        }

        // Game logic goes here.
		if (kbState[SDL_SCANCODE_ESCAPE]) {
			shouldExit = 1;
		}
		if (kbState[SDL_SCANCODE_UP]) {
			spritePos[1] -= 300 * deltaTime;
		}
		if (kbState[SDL_SCANCODE_DOWN]) {
			spritePos[1] += 300 * deltaTime;
		}
		if (kbState[SDL_SCANCODE_LEFT]) {
			spritePos[0] -= 300 * deltaTime;
		}
		if (kbState[SDL_SCANCODE_RIGHT]) {
			spritePos[0] += 300 * deltaTime;
		}

		if (kbState[SDL_SCANCODE_W]) {
			camPos[1] = -1;
		} else if (kbState[SDL_SCANCODE_S]) {
			camPos[1] = 1;
		} else { camPos[1] = 0; }

		if (kbState[SDL_SCANCODE_A]) {
			camPos[0] = -1;
		} else if (kbState[SDL_SCANCODE_D]) {
			camPos[0] = 1;
		} else { camPos[0] = 0; }
		
		camera->Move(deltaTime, camPos);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

		// Game drawing goes here.
		camera->Draw(deltaTime);
        glDrawSprite(spriteTex, spritePos[0], spritePos[1], spriteSize[0], spriteSize[1]);

        // Present the most recent frame.
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}
