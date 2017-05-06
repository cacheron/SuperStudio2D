#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "DrawUtils.h"
#include "Rendering.h"
#include "Physics.h"

// Set this to true to make the game loop exit.
char shouldExit = 0;

// The previous frame's keyboard state.
unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};

// The current frame's keyboard state.
const unsigned char* kbState = NULL;

// Camera pos
int camInput[2];

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
            "Overbooked",
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

	// Calculation of deltaTime
	Uint64 NOW = SDL_GetTicks();
	Uint64 LAST = 0;
	Uint64 elapsedTime = 0;
	float deltaTime, seconds = 0;

	// Create the background
	int tex_w = 0; int tex_h = 0;
	int tex_broke = glTexImageTGAFile("img/tex_broke.tga", &tex_w, &tex_h);
	Tile* broke = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_broke, false);

	int tex_clouds = glTexImageTGAFile("img/tex_clouds.tga", &tex_w, &tex_h);
	Tile* clouds = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_clouds, false);

	int tex_carpet = glTexImageTGAFile("img/tex_carpet.tga", &tex_w, &tex_h);
	Tile* carpet = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_carpet, false);

	int level[40][13] = {	{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 },
							{ 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1 }
						};
	// convert level to 2d vector
	vector<vector<int>> vectorLevel;
	for (int i = 0; i < 40; ++i) {
		vectorLevel.push_back(vector<int>(begin(level[i]), end(level[i])));
	}
	Background* bg = new Background(13, 40);
	bg->SetLevel(vectorLevel);
	bg->AddToTileSet(broke);
	bg->AddToTileSet(clouds);
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

	int decorations[40][13] = { { 0, 1, 0, 0, 0, 0, 0, 0, 4, 4, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 0, 0, 0, 3, 2, 0 },
								{ 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
								{ 0, 1, 3, 3, 3, 3, 3, 3, 0, 3, 4, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 4, 4, 4, 4, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 4, 4, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 0, 0, 3, 3, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 3, 0, 0, 3, 3, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 3, 3, 3, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 4, 4, 4, 4, 4, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 4, 0, 4, 4, 4, 4, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
								{ 0, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0 }
	};
	vector<vector<int>> vectordeco;
	for (int i = 0; i < 40; i++) {
		vectordeco.push_back(vector<int>(begin(decorations[i]), end(decorations[i])));
	}
	Background* decoration = new Background(13, 40);
	decoration->SetLevel(vectordeco);
	decoration->AddToTileSet(transparent);
	decoration->AddToTileSet(window_left);
	decoration->AddToTileSet(window_right);
	decoration->AddToTileSet(chair);
	decoration->AddToTileSet(red_case);
	
	// Create the player animations
		vector<Frame*> frames_player_up;
		vector<Frame*> frames_player_down;
		vector<Frame*> frames_player_left;
		vector<Frame*> frames_player_right;
		vector<Frame*> frames_player_dead;
		int spr_player_up = glTexImageTGAFile("img/spr_player_back.tga", &tex_w, &tex_h);
		int spr_player_down = glTexImageTGAFile("img/spr_player_front.tga", &tex_w, &tex_h);
		int spr_player_left = glTexImageTGAFile("img/spr_player_left.tga", &tex_w, &tex_h);
		int spr_player_right = glTexImageTGAFile("img/spr_player_right.tga", &tex_w, &tex_h);
		int spr_player_dead = glTexImageTGAFile("img/spr_player_dead.tga", &tex_w, &tex_h);

		// putting false because the actor has a boxcollider, TODO use frame colliders instead of actor
		Frame* player_up = new Frame(0, 0, 64, 64, spr_player_up, false, 0.2);		frames_player_up.push_back(player_up);
		Frame* player_down = new Frame(0, 0, 64, 64, spr_player_down, false, 0.2);	frames_player_down.push_back(player_down);
		Frame* player_left = new Frame(0, 0, 64, 64, spr_player_left, false, 0.2);	frames_player_left.push_back(player_left);
		Frame* player_right = new Frame(0, 0, 64, 64, spr_player_right, false, 0.2); frames_player_right.push_back(player_right);
		Frame* player_dead = new Frame(0, 0, 64, 64, spr_player_dead, false, 0.2); frames_player_dead.push_back(player_dead);

		Animation* anim_player_up = new Animation(frames_player_up, false, true);
		Animation* anim_player_down = new Animation(frames_player_down, false, true);
		Animation* anim_player_left = new Animation(frames_player_left, false, true);
		Animation* anim_player_right = new Animation(frames_player_right, false, true);
		Animation* anim_player_dead = new Animation(frames_player_dead, false, true);
	// Create the actor
	Actor* player = new Actor(128, 0, 64, 64, 20, 350, 5);
	player->AddAnimation(anim_player_up);
	player->AddAnimation(anim_player_down);
	player->AddAnimation(anim_player_left);
	player->AddAnimation(anim_player_right);
	player->AddAnimation(anim_player_dead);

	// Create the AI
	// Create the animations
		vector<Frame*> frames_attendant_up;
		vector<Frame*> frames_attendant_down;
		vector<Frame*> frames_attendant_left;
		vector<Frame*> frames_attendant_right;
		int spr_peanut = glTexImageTGAFile("img/spr_peanut.tga", &tex_w, &tex_h);
		int spr_attendant_up = glTexImageTGAFile("img/spr_attendant_back.tga", &tex_w, &tex_h);
		int spr_attendant_down = glTexImageTGAFile("img/spr_attendant_front.tga", &tex_w, &tex_h);
		int spr_attendant_left = glTexImageTGAFile("img/spr_attendant_left.tga", &tex_w, &tex_h);
		int spr_attendant_right = glTexImageTGAFile("img/spr_attendant_right.tga", &tex_w, &tex_h);

		// putting false because the actor has a boxcollider, TODO use frame colliders instead of actor
		Frame* attendant_up = new Frame(0, 0, 64, 64, spr_attendant_up, false, 0.2);		frames_attendant_up.push_back(attendant_up);
		Frame* attendant_down = new Frame(0, 0, 64, 64, spr_attendant_down, false, 0.2);	frames_attendant_down.push_back(attendant_down);
		Frame* attendant_left = new Frame(0, 0, 64, 64, spr_attendant_left, false, 0.2);	frames_attendant_left.push_back(attendant_left);
		Frame* attendant_right = new Frame(0, 0, 64, 64, spr_attendant_right, false, 0.2);  frames_attendant_right.push_back(attendant_right);

		Animation* anim_attendant_up = new Animation(frames_attendant_up, false, true);
		Animation* anim_attendant_down = new Animation(frames_attendant_down, false, true);
		Animation* anim_attendant_left = new Animation(frames_attendant_left, false, true);
		Animation* anim_attendant_right = new Animation(frames_attendant_right, false, true);

	// Instantiate AI (equal behavior)
		Sentry* sentry = new Sentry(2*64, 7*64, 64, 64, 5, 90, 3*64, 4);
		// add the peanut proj
		sentry->peanut->img = spr_peanut;
		sentry->AddAnimation(anim_attendant_up);
		sentry->AddAnimation(anim_attendant_down);
		sentry->AddAnimation(anim_attendant_left);
		sentry->AddAnimation(anim_attendant_right);
		// Vector path
		vector<int*> path; path.reserve(2);
		int point1[2] = {9, 7}; int point2[2] = {2, 7};
		path.push_back(point1); path.push_back(point2);
		sentry->SetPath(path);

	// Instantiate AI 2 (close range, mostly chases and shoots / runs some of the time)
		Sentry* sentry2 = new Sentry(9 * 64, 20 * 64, 64, 64, 5, 90, 3 * 64, 4);
		// add the peanut proj
		sentry2->peanut->img = spr_peanut;
		sentry2->AddAnimation(anim_attendant_up);
		sentry2->AddAnimation(anim_attendant_down);
		sentry2->AddAnimation(anim_attendant_left);
		sentry2->AddAnimation(anim_attendant_right);
		// Vector path
		vector<int*> path2; path2.reserve(4);
		int s2_p1[2] = { 9, 20 }; int s2_p2[2] = { 9, 28 };
		int s2_p3[2] = { 2, 28 }; int s2_p4[2] = { 2, 22 };
		path2.push_back(s2_p1); path2.push_back(s2_p2);
		path2.push_back(s2_p3);	path2.push_back(s2_p4);
		sentry2->SetPath(path2);
		sentry2->SetBehavior(0.75, 0.125, 0.125);
	// Instantiate AI 3 (ranger, mostly runs and shoots)
		Sentry* sentry3 = new Sentry(3 * 64, 31 * 64, 64, 64, 5, 90, 3 * 64, 4);
		// add the peanut proj
		sentry3->peanut->img = spr_peanut;
		sentry3->AddAnimation(anim_attendant_up);
		sentry3->AddAnimation(anim_attendant_down);
		sentry3->AddAnimation(anim_attendant_left);
		sentry3->AddAnimation(anim_attendant_right);
		// Vector path
		vector<int*> path3; path3.reserve(6);
		int s3_p1[2] = { 3, 31 }; int s3_p2[2] = { 8, 31 };
		int s3_p3[2] = { 3, 37 }; int s3_p4[2] = { 8, 37 };
		path3.push_back(s3_p1); path3.push_back(s3_p2); path3.push_back(s3_p1);
		path3.push_back(s3_p3);	path3.push_back(s3_p4); path3.push_back(s3_p3);
		sentry3->SetPath(path3);
		sentry3->SetBehavior(0.1, 0.25, 0.65);
	// Add all entites to physics
	Physics* physics = new Physics();
	physics->AddToPhysicsUpdate(player);
	physics->AddToPhysicsUpdate(sentry);
	physics->AddToPhysicsUpdate(sentry2);
	physics->AddToPhysicsUpdate(sentry3);
	physics->AddToPhysicsUpdate(decoration);
	physics->AddToPhysicsUpdate(sentry->peanut);
	physics->AddToPhysicsUpdate(sentry2->peanut);
	physics->AddToPhysicsUpdate(sentry3->peanut);
	// Add all entities to game loop
	Camera* camera = new Camera(0, 0, 14, 11, 300);
	camera->AddBackground(*bg);
	camera->AddDecoration(*decoration);
	camera->AddActor(player);
	camera->AddActor(sentry);
	camera->AddActor(sentry2);
	camera->AddActor(sentry3);
	camera->AddProjectile(sentry->peanut);
	camera->AddProjectile(sentry2->peanut);
	camera->AddProjectile(sentry3->peanut);

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
		if (kbState[SDL_SCANCODE_SPACE]) {
			printf(" (%d, %d)", player->x / 64, player->y / 64);
			system("pause");
		}
		// player input
		if (kbState[SDL_SCANCODE_UP]) {
			player->input[1] = -1;
		} else if (kbState[SDL_SCANCODE_DOWN]) {
			player->input[1] = 1;
		} else { player->input[1] = 0; }

		if (kbState[SDL_SCANCODE_LEFT]) {
			player->input[0] = -1;
		} else if (kbState[SDL_SCANCODE_RIGHT]) {
			player->input[0] = 1;
		} else { player->input[0] = 0; }

		if (kbState[SDL_SCANCODE_W]) {
			camInput[1] = -1;
		} else if (kbState[SDL_SCANCODE_S]) {
			camInput[1] = 1;
		} else { camInput[1] = 0; }
		camInput[0] = 0;

		camera->Move(deltaTime, camInput);
		player->Update(deltaTime);
		sentry->SetTarget(player->x, player->y);
		sentry2->SetTarget(player->x, player->y);
		sentry3->SetTarget(player->x, player->y);
		sentry->Update(deltaTime);
		sentry2->Update(deltaTime);
		sentry3->Update(deltaTime);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

		// Handle physics
		physics->Update(deltaTime);
		physics->DetectCollisions();
		// Game drawing goes here.
		camera->Draw(deltaTime);

        // Present the most recent frame.
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}
