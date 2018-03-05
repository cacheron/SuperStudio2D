#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "DrawUtils.h"
#include "Rendering.h"
#include "Physics.h"
#include "Level.h"
#include "fmod.h"

// Set this to true to make the game loop exit.
char shouldExit = 0;
// The previous frame's keyboard state.
unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};
// The current frame's keyboard state.
const unsigned char* kbState = NULL;

int main(void) {
      //////////////////////////////////////////////////////////////////////////
	 /*                       Initialize SDL                                 */	
	//////////////////////////////////////////////////////////////////////////
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
	SDL_GL_SetSwapInterval(1);
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

	  //////////////////////////////////////////////////////////////////////////
	 /*                Initialize Sound and FMOD                             */
	//////////////////////////////////////////////////////////////////////////
	FMOD_SYSTEM* fmod; FMOD_CHANNEL *titleChan, *mainChan, *soundChan;
	FMOD_System_Create(&fmod);
	FMOD_System_Init(fmod, 100, FMOD_INIT_NORMAL, 0);

	FMOD_SOUND *stream_titletheme, *stream_maintheme; // music
	FMOD_System_CreateStream(fmod, "sound/stream_titletheme.mp3", FMOD_DEFAULT, 0, &stream_titletheme);
	FMOD_System_CreateStream(fmod, "sound/stream_maintheme.mp3", FMOD_DEFAULT, 0, &stream_maintheme);

	FMOD_SOUND *sound_gameover, *sound_fanfare;
	FMOD_System_CreateSound(fmod, "sound/sound_gameover.mp3", FMOD_DEFAULT, 0, &sound_gameover);
	FMOD_System_CreateSound(fmod, "sound/sound_fanfare.mp3", FMOD_DEFAULT, 0, &sound_fanfare);

	  //////////////////////////////////////////////////////////////////////////
	 /*                Initialize Game Levels and Tiles                      */
	//////////////////////////////////////////////////////////////////////////

	// generic container for texture heights and widths
	int tex_w = 0; int tex_h = 0;
	
	// filter images + ui
	int filter_crt = glTexImageTGAFile("img/filter/filter_crt.tga", &tex_w, &tex_h);
	int filter_vignette = glTexImageTGAFile("img/filter/filter_vignette.tga", &tex_w, &tex_h);
	int spr_gameover = glTexImageTGAFile("img/ui/spr_gameover.tga", &tex_w, &tex_h);
	int spr_rec = glTexImageTGAFile("img/ui/spr_record.tga", &tex_w, &tex_h);
	int spr_title = glTexImageTGAFile("img/ui/spr_title.tga", &tex_w, &tex_h);
	int spr_tryagain = glTexImageTGAFile("img/ui/spr_tryagain.tga", &tex_w, &tex_h);
	int spr_crawl_bg = glTexImageTGAFile("img/ui/spr_crawl_bg.tga", &tex_w, &tex_h);
	int spr_crawl_text = glTexImageTGAFile("img/ui/spr_crawl_text.tga", &tex_w, &tex_h);
	int spr_endscreen = glTexImageTGAFile("img/ui/spr_endscreen.tga", &tex_w, &tex_h);
	int spr_instructions = glTexImageTGAFile("img/ui/spr_instruction.tga", &tex_w, &tex_h);
	// Background textures
	int tex_broke = glTexImageTGAFile("img/tex_broke.tga", &tex_w, &tex_h);
	Tile* broke = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_broke, false);

	int tex_clouds = glTexImageTGAFile("img/env/tex_clouds.tga", &tex_w, &tex_h);
	Tile* clouds = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_clouds, false);

	int tex_sundown = glTexImageTGAFile("img/env/tex_sundown.tga", &tex_w, &tex_h);
	Tile* sundown = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_sundown, false);

	int tex_night = glTexImageTGAFile("img/env/tex_night.tga", &tex_w, &tex_h);
	Tile* night = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_night, false);

	int tex_carpet = glTexImageTGAFile("img/env/tex_carpet.tga", &tex_w, &tex_h);
	Tile* carpet = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_carpet, false);

	int tex_metal = glTexImageTGAFile("img/env/tex_metal.tga", &tex_w, &tex_h);
	Tile* metal = new Tile(0.0f, 0.0f, tex_w, tex_h, tex_metal, false);

	// Decorations sprites
	int spr_transparent = glTexImageTGAFile("img/env/spr_transparent.tga", &tex_w, &tex_h);
	Tile* transparent = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_transparent, false);

	int spr_window_left = glTexImageTGAFile("img/env/spr_window_left.tga", &tex_w, &tex_h);
	Tile* window_left = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_window_left, true);

	int spr_window_right = glTexImageTGAFile("img/env/spr_window_right.tga", &tex_w, &tex_h);
	Tile* window_right = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_window_right, true);

	int spr_chair_person = glTexImageTGAFile("img/env/spr_chair_person.tga", &tex_w, &tex_h);
	Tile* chair_person = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_chair_person, true);

	int spr_case = glTexImageTGAFile("img/env/spr_case.tga", &tex_w, &tex_h);
	Tile* red_case = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_case, true);

	int spr_box = glTexImageTGAFile("img/env/spr_box.tga", &tex_w, &tex_h);
	Tile* box = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_box, true);

	int spr_box_blue = glTexImageTGAFile("img/env/spr_box_blue.tga", &tex_w, &tex_h);
	Tile* box_blue = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_box_blue, true);

	int spr_stairs_down = glTexImageTGAFile("img/env/spr_stairs_down.tga", &tex_w, &tex_h);
	Tile* stairs_down = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_stairs_down, true);

	int spr_stairs_up = glTexImageTGAFile("img/env/spr_stairs_up.tga", &tex_w, &tex_h);
	Tile* stairs_up = new Tile(0.0f, 0.0f, tex_w, tex_h, spr_stairs_up, true);
	
	int spr_ticket = glTexImageTGAFile("img/spr_ticket.tga", &tex_w, &tex_h);
	int spr_chair = glTexImageTGAFile("img/env/spr_chair.tga", &tex_w, &tex_h);

	// Create the universal tileset for background and deco
	vector<Tile*> textures, decorations;
	textures.push_back(broke);
	textures.push_back(clouds);
	textures.push_back(sundown);
	textures.push_back(night);
	textures.push_back(carpet);
	textures.push_back(metal);
	
	decorations.push_back(transparent);
	decorations.push_back(window_left);
	decorations.push_back(window_right);
	decorations.push_back(chair_person);
	decorations.push_back(red_case);
	decorations.push_back(box);
	decorations.push_back(box_blue);
	decorations.push_back(stairs_down);
	decorations.push_back(stairs_up);

	  //////////////////////////////////////////////////////////////////////////
	 /*                       Initialize Character Animations                */
	//////////////////////////////////////////////////////////////////////////

	// Player animations
	// Movement
	int spr_player_up = glTexImageTGAFile("img/player/spr_player_back.tga", &tex_w, &tex_h);
	int spr_player_down = glTexImageTGAFile("img/player/spr_player_front.tga", &tex_w, &tex_h);
	int spr_player_left = glTexImageTGAFile("img/player/spr_player_left.tga", &tex_w, &tex_h);
	int spr_player_right = glTexImageTGAFile("img/player/spr_player_right.tga", &tex_w, &tex_h);
	int spr_player_dead = glTexImageTGAFile("img/player/spr_player_dead.tga", &tex_w, &tex_h);
	int spr_player_hit = glTexImageTGAFile("img/player/spr_player_hit.tga", &tex_w, &tex_h);
	vector<Frame*> frames_player_up, frames_player_down, frames_player_left, frames_player_right, frames_player_dead, frames_player_hit;
	Frame* player_up = new Frame(0, 0, 64, 64, spr_player_up, false, 0.2);			frames_player_up.push_back(player_up);
	Frame* player_down = new Frame(0, 0, 64, 64, spr_player_down, false, 0.2);		frames_player_down.push_back(player_down);
	Frame* player_left = new Frame(0, 0, 64, 64, spr_player_left, false, 0.2);		frames_player_left.push_back(player_left);
	Frame* player_right = new Frame(0, 0, 64, 64, spr_player_right, false, 0.2);	frames_player_right.push_back(player_right);
	Frame* player_dead = new Frame(0, 0, 64, 64, spr_player_dead, false, 0.2);		frames_player_dead.push_back(player_dead);
	Frame* player_hit = new Frame(0, 0, 64, 64, spr_player_hit, false, 0.2);		frames_player_hit.push_back(player_hit);
	Animation* anim_player_up = new Animation(frames_player_up, false, true);
	Animation* anim_player_down = new Animation(frames_player_down, false, true);
	Animation* anim_player_left = new Animation(frames_player_left, false, true);
	Animation* anim_player_right = new Animation(frames_player_right, false, true);
	Animation* anim_player_dead = new Animation(frames_player_dead, false, true);
	Animation* anim_player_hit = new Animation(frames_player_hit, false, false);
	
	// Punch Animations and proj
	int spr_player_punch_up = glTexImageTGAFile("img/player/spr_player_punch_back.tga", &tex_w, &tex_h);
	int spr_player_punch_down = glTexImageTGAFile("img/player/spr_player_punch_front.tga", &tex_w, &tex_h);
	int spr_player_punch_left = glTexImageTGAFile("img/player/spr_player_punch_left.tga", &tex_w, &tex_h);
	int spr_player_punch_right = glTexImageTGAFile("img/player/spr_player_punch_right.tga", &tex_w, &tex_h);
	int spr_punch = glTexImageTGAFile("img/player/spr_punch.tga", &tex_w, &tex_h);
	vector<Frame*> frames_player_punch_up, frames_player_punch_down, frames_player_punch_left, frames_player_punch_right;
	Frame* player_punch_up = new Frame(0, 0, 64, 64, spr_player_punch_up, false, 0.18);			frames_player_punch_up.push_back(player_punch_up);
	Frame* player_punch_down = new Frame(0, 0, 64, 64, spr_player_punch_down, false, 0.18);		frames_player_punch_down.push_back(player_punch_down);
	Frame* player_punch_left = new Frame(0, 0, 64, 64, spr_player_punch_left, false, 0.18);		frames_player_punch_left.push_back(player_punch_left);
	Frame* player_punch_right = new Frame(0, 0, 64, 64, spr_player_punch_right, false, 0.18);	frames_player_punch_right.push_back(player_punch_right);
	Animation* anim_player_punch_up = new Animation(frames_player_punch_up, false, false);
	Animation* anim_player_punch_down = new Animation(frames_player_punch_down, false, false);
	Animation* anim_player_punch_left = new Animation(frames_player_punch_left, false, false);
	Animation* anim_player_punch_right = new Animation(frames_player_punch_right, false, false);

	// Sentry Animations
	int spr_peanut = glTexImageTGAFile("img/spr_peanut.tga", &tex_w, &tex_h);
	int spr_attendant_up = glTexImageTGAFile("img/sentry/spr_attendant_back.tga", &tex_w, &tex_h);
	int spr_attendant_down = glTexImageTGAFile("img/sentry/spr_attendant_front.tga", &tex_w, &tex_h);
	int spr_attendant_left = glTexImageTGAFile("img/sentry/spr_attendant_left.tga", &tex_w, &tex_h);
	int spr_attendant_right = glTexImageTGAFile("img/sentry/spr_attendant_right.tga", &tex_w, &tex_h);
	int spr_attendant_dead = glTexImageTGAFile("img/sentry/spr_attendant_dead.tga", &tex_w, &tex_h);
	int spr_attendant_hit = glTexImageTGAFile("img/sentry/spr_attendant_hit.tga", &tex_w, &tex_h);
	vector<Frame*> frames_attendant_up, frames_attendant_down, frames_attendant_left, frames_attendant_right, frames_attendant_dead, frames_attendant_hit;
	Frame* attendant_up = new Frame(0, 0, 64, 64, spr_attendant_up, false, 0.2);		frames_attendant_up.push_back(attendant_up);
	Frame* attendant_down = new Frame(0, 0, 64, 64, spr_attendant_down, false, 0.2);	frames_attendant_down.push_back(attendant_down);
	Frame* attendant_left = new Frame(0, 0, 64, 64, spr_attendant_left, false, 0.2);	frames_attendant_left.push_back(attendant_left);
	Frame* attendant_right = new Frame(0, 0, 64, 64, spr_attendant_right, false, 0.2);  frames_attendant_right.push_back(attendant_right);
	Frame* attendant_dead = new Frame(0, 0, 64, 64, spr_attendant_dead, false, 0.2);	frames_attendant_dead.push_back(attendant_dead);
	Frame* attendant_hit = new Frame(0, 0, 64, 64, spr_attendant_hit, false, 0.2);	frames_attendant_hit.push_back(attendant_hit);
	Animation* anim_attendant_up = new Animation(frames_attendant_up, false, true);
	Animation* anim_attendant_down = new Animation(frames_attendant_down, false, true);
	Animation* anim_attendant_left = new Animation(frames_attendant_left, false, true);
	Animation* anim_attendant_right = new Animation(frames_attendant_right, false, true);
	Animation* anim_attendant_dead = new Animation(frames_attendant_dead, false, true);
	Animation* anim_attendant_hit = new Animation(frames_attendant_hit, false, true);

	// ranger Animations
	int spr_peanut_bag = glTexImageTGAFile("img/spr_peanut_bag.tga", &tex_w, &tex_h);
	int spr_ranger_up = glTexImageTGAFile("img/ranger/spr_ranger_back.tga", &tex_w, &tex_h);
	int spr_ranger_down = glTexImageTGAFile("img/ranger/spr_ranger_front.tga", &tex_w, &tex_h);
	int spr_ranger_left = glTexImageTGAFile("img/ranger/spr_ranger_left.tga", &tex_w, &tex_h);
	int spr_ranger_right = glTexImageTGAFile("img/ranger/spr_ranger_right.tga", &tex_w, &tex_h);
	int spr_ranger_dead = glTexImageTGAFile("img/ranger/spr_ranger_dead.tga", &tex_w, &tex_h);
	int spr_ranger_hit = glTexImageTGAFile("img/ranger/spr_ranger_hit.tga", &tex_w, &tex_h);
	vector<Frame*> frames_ranger_up, frames_ranger_down, frames_ranger_left, frames_ranger_right, frames_ranger_dead, frames_ranger_hit;
	Frame* ranger_up = new Frame(0, 0, 64, 64, spr_ranger_up, false, 0.2);		frames_ranger_up.push_back(ranger_up);
	Frame* ranger_down = new Frame(0, 0, 64, 64, spr_ranger_down, false, 0.2);	frames_ranger_down.push_back(ranger_down);
	Frame* ranger_left = new Frame(0, 0, 64, 64, spr_ranger_left, false, 0.2);	frames_ranger_left.push_back(ranger_left);
	Frame* ranger_right = new Frame(0, 0, 64, 64, spr_ranger_right, false, 0.2);  frames_ranger_right.push_back(ranger_right);
	Frame* ranger_dead = new Frame(0, 0, 64, 64, spr_ranger_dead, false, 0.2);	frames_ranger_dead.push_back(ranger_dead);
	Frame* ranger_hit = new Frame(0, 0, 64, 64, spr_ranger_hit, false, 0.2);	frames_ranger_hit.push_back(ranger_hit);
	Animation* anim_ranger_up = new Animation(frames_ranger_up, false, true);
	Animation* anim_ranger_down = new Animation(frames_ranger_down, false, true);
	Animation* anim_ranger_left = new Animation(frames_ranger_left, false, true);
	Animation* anim_ranger_right = new Animation(frames_ranger_right, false, true);
	Animation* anim_ranger_dead = new Animation(frames_ranger_dead, false, true);
	Animation* anim_ranger_hit = new Animation(frames_ranger_hit, false, true);

	// Scorpion Animations
	int spr_scorp_up = glTexImageTGAFile("img/scorpion/spr_scorpion_back.tga", &tex_w, &tex_h);
	int spr_scorp_down = glTexImageTGAFile("img/scorpion/spr_scorpion_front.tga", &tex_w, &tex_h);
	int spr_scorp_left = glTexImageTGAFile("img/scorpion/spr_scorpion_left.tga", &tex_w, &tex_h);
	int spr_scorp_right = glTexImageTGAFile("img/scorpion/spr_scorpion_right.tga", &tex_w, &tex_h);
	int spr_scorp_dead = glTexImageTGAFile("img/scorpion/spr_scorpion_dead.tga", &tex_w, &tex_h);
	int spr_scorp_hit = glTexImageTGAFile("img/scorpion/spr_scorpion_hit.tga", &tex_w, &tex_h);
	vector<Frame*> frames_scorp_up, frames_scorp_down, frames_scorp_left, frames_scorp_right, frames_scorp_dead, frames_scorp_hit;
	Frame* scorp_up = new Frame(0, 0, 64, 64, spr_scorp_up, false, 0.2);		frames_scorp_up.push_back(scorp_up);
	Frame* scorp_down = new Frame(0, 0, 64, 64, spr_scorp_down, false, 0.2);	frames_scorp_down.push_back(scorp_down);
	Frame* scorp_left = new Frame(0, 0, 64, 64, spr_scorp_left, false, 0.2);	frames_scorp_left.push_back(scorp_left);
	Frame* scorp_right = new Frame(0, 0, 64, 64, spr_scorp_right, false, 0.2);  frames_scorp_right.push_back(scorp_right);
	Frame* scorp_dead = new Frame(0, 0, 64, 64, spr_scorp_dead, false, 0.2);	frames_scorp_dead.push_back(scorp_dead);
	Frame* scorp_hit = new Frame(0, 0, 64, 64, spr_scorp_hit, false, 0.2);	frames_scorp_hit.push_back(scorp_hit);
	Animation* anim_scorp_up = new Animation(frames_scorp_up, false, true);
	Animation* anim_scorp_down = new Animation(frames_scorp_down, false, true);
	Animation* anim_scorp_left = new Animation(frames_scorp_left, false, true);
	Animation* anim_scorp_right = new Animation(frames_scorp_right, false, true);
	Animation* anim_scorp_dead = new Animation(frames_scorp_dead, false, true);
	Animation* anim_scorp_hit = new Animation(frames_scorp_hit, false, true);

	  //////////////////////////////////////////////////////////////////////////
	 /*                       Create the basic enemies                       */
	//////////////////////////////////////////////////////////////////////////
	// Create The general enemies for all levels
	// NOTE: All enemies MUST have a path if you want them to patrol
	// Generic Sentry (equal behavior)
	Sentry* sentry = new Sentry(0, 0, 64, 64, 3, 120, 3.5 * 64, 5);
	sentry->peanut->img = spr_peanut;
	sentry->AddAnimation(anim_attendant_up);
	sentry->AddAnimation(anim_attendant_down);
	sentry->AddAnimation(anim_attendant_left);
	sentry->AddAnimation(anim_attendant_right);
	sentry->AddAnimation(anim_attendant_dead);
	sentry->AddAnimation(anim_attendant_hit);
	sentry->SetBehavior(0.55, 0.1, 0.35);

	// Scorpion Enemy (chase at fast speeds)
	Sentry* scorpion = new Sentry(0, 0, 64, 64, 2, 200, 5 * 64, 5);
	scorpion->peanut->img = spr_peanut; // TODO change peanut to different proj for scorp
	scorpion->AddAnimation(anim_scorp_up);
	scorpion->AddAnimation(anim_scorp_down);
	scorpion->AddAnimation(anim_scorp_left);
	scorpion->AddAnimation(anim_scorp_right);
	scorpion->AddAnimation(anim_scorp_dead);
	scorpion->AddAnimation(anim_scorp_hit);
	scorpion->SetBehavior(1.0, 0.0, 0.0);

	// Ranged Sentry, usually shoots or runs
	Sentry* ranger = new Sentry(0, 0, 64, 64, 5, 90, 4 * 64, 5);
	ranger->peanut->img = spr_peanut_bag;
	ranger->peanut->damage = 8;
	ranger->AddAnimation(anim_ranger_up);
	ranger->AddAnimation(anim_ranger_down);
	ranger->AddAnimation(anim_ranger_left);
	ranger->AddAnimation(anim_ranger_right);
	ranger->AddAnimation(anim_ranger_dead);
	ranger->AddAnimation(anim_ranger_hit);
	ranger->SetBehavior(0.1, 0.25, 0.65);

	  //////////////////////////////////////////////////////////////////////////
	 /*                       Initialize Levels                              */
	//////////////////////////////////////////////////////////////////////////
	// Global level variables
	Level* currentLevel = new Level();

	// LEVEL 1
		Level* level1 = new Level();
		int level1_int_bg[60][13] = {
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },

			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },

			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 },
			{ 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1 }
		};
		int level1_int_deco[60][13] = {
			{ 0, 1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 4, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 4, 4, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },

			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 5, 5, 5, 5, 5, 0, 0, 5, 5, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 5, 6, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 6, 5, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 5, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 5, 5, 0, 2, 0 },
			{ 0, 1, 5, 5, 5, 0, 0, 0, 5, 5, 0, 2, 0 },
			{ 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 5, 5, 5, 0, 0, 0, 0, 5, 5, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 4, 4, 2, 0 },

			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 3, 3, 0, 3, 3, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0 }
		};

		// Assign the background and decorations for Level 1
		vector<vector<int>> vector_lvl1_bg, vector_lvl1_deco;
		for (int i = 0; i < 60; ++i) { vector_lvl1_bg.push_back(vector<int>(begin(level1_int_bg[i]), end(level1_int_bg[i]))); }
		for (int i = 0; i < 60; ++i) { vector_lvl1_deco.push_back(vector<int>(begin(level1_int_deco[i]), end(level1_int_deco[i]))); }
		Background* level1_bg = new Background(13, 60); level1_bg->SetLevel(vector_lvl1_bg);
		Background* level1_deco = new Background(13, 60); level1_deco->SetLevel(vector_lvl1_deco);
		level1_bg->SetTileSet(textures);
		level1_deco->SetTileSet(decorations);
		level1->background = level1_bg;
		level1->decorations = level1_deco;
		// put the gate key in the level
		level1->gate_key->img = spr_ticket;
		level1->gate_key->Move(6 * 64, 37 * 64);
		level1->gate_key_x = 6 * 64;
		level1->gate_key_y = 37 * 64;
		level1->gate->img = spr_stairs_down;
		level1->gate_x = 2 * 64; level1->gate_y = 24 * 64;
		
		// Create the enemies for level 1
		// sentry 1
		Sentry* lvl1_sentry = new Sentry();
		lvl1_sentry->CopyValues(sentry);
		lvl1_sentry->Move(3 * 64, 5 * 64);
		vector<int*> lvl1_path; lvl1_path.reserve(2);
		int point1[2] = { 3, 5 },
			point2[2] = { 8, 5 };
		lvl1_path.push_back(point1); lvl1_path.push_back(point2);
		lvl1_sentry->SetPath(lvl1_path);

		// sentry 2
		Sentry* lvl1_sentry2 = new Sentry();
		lvl1_sentry2->CopyValues(sentry);
		lvl1_sentry2->Move(9 * 64, 10 * 64);
		vector<int*> lvl1_path2; lvl1_path2.reserve(4);
		int	lvl1_s2_p1[2] = { 9, 10 },
			lvl1_s2_p2[2] = { 6, 10 },
			lvl1_s2_p3[2] = { 6, 14 },
			lvl1_s2_p4[2] = { 9, 14 };
		lvl1_path2.push_back(lvl1_s2_p1); 
		lvl1_path2.push_back(lvl1_s2_p2);
		lvl1_path2.push_back(lvl1_s2_p3);
		lvl1_path2.push_back(lvl1_s2_p4);
		lvl1_sentry2->SetPath(lvl1_path2);
		lvl1_sentry2->SetPath(lvl1_path2);

		// sentry 3
		Sentry* lvl1_sentry3 = new Sentry();
		lvl1_sentry3->CopyValues(sentry);
		lvl1_sentry3->Move(6 * 64, 18 * 64);
		vector<int*> lvl1_path3; lvl1_path3.reserve(2);
		int	lvl1_s3_p1[2] = { 6, 18 },
			lvl1_s3_p2[2] = { 6, 25 };
		lvl1_path3.push_back(lvl1_s3_p1);
		lvl1_path3.push_back(lvl1_s3_p2);
		lvl1_sentry3->SetPath(lvl1_path3);
		lvl1_sentry3->SetPath(lvl1_path3);

		// ranger 1
		Sentry* lvl1_ranger = new Sentry();
		lvl1_ranger->CopyValues(ranger);
		lvl1_ranger->Move(8 * 64, 30 * 64);
		vector<int*> lvl1_path4; lvl1_path4.reserve(4);
		int lvl1_r1_p1[2] = { 8, 30 },
			lvl1_r1_p2[2] = { 3, 31 },
			lvl1_r1_p3[2] = { 3, 37 },
			lvl1_r1_p4[2] = { 9, 37 };
		lvl1_path4.push_back(lvl1_r1_p1);
		lvl1_path4.push_back(lvl1_r1_p2);
		lvl1_path4.push_back(lvl1_r1_p3);
		lvl1_path4.push_back(lvl1_r1_p4);
		lvl1_ranger->SetPath(lvl1_path4);

		// give them a vector path
		level1->enemies.push_back(lvl1_sentry); 
		level1->enemies.push_back(lvl1_sentry2); 
		level1->enemies.push_back(lvl1_sentry3);
		level1->enemies.push_back(lvl1_ranger);
		
	// LEVEL 2
		Level* level2 = new Level();
		int level2_int_bg[60][13] = {
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },

			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },

			{ 2, 2, 4, 4, 4, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 4, 4, 4, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 4, 4, 4, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 },
			{ 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2 }
		};
		int level2_int_deco[60][13] = {
			{ 0, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0 },
			{ 0, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 0 },
			{ 0, 1, 0, 0, 4, 4, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 4, 4, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 4, 4, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 4, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 4, 2, 0 },

			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 4, 4, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 5, 5, 5, 5, 5, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 5, 5, 5, 5, 5, 0, 0, 6, 6, 2, 0 },

			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 6, 6, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 6, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 6, 6, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 0 }
		};
		// Assign the background and decorations for Level 2
		vector<vector<int>> vector_lvl2_bg, vector_lvl2_deco;
		for (int i = 0; i < 60; ++i) { vector_lvl2_bg.push_back(vector<int>(begin(level2_int_bg[i]), end(level2_int_bg[i]))); }
		for (int i = 0; i < 60; ++i) { vector_lvl2_deco.push_back(vector<int>(begin(level2_int_deco[i]), end(level2_int_deco[i]))); }
		Background* level2_bg = new Background(13, 60); level2_bg->SetLevel(vector_lvl2_bg);
		Background* level2_deco = new Background(13, 60); level2_deco->SetLevel(vector_lvl2_deco);
		level2_bg->SetTileSet(textures);
		level2_deco->SetTileSet(decorations);
		level2->background = level2_bg;
		level2->decorations = level2_deco;
		
		// Level 2 gate key
		level2->gate_key->img = spr_ticket;
		level2->gate_key->Move(9 * 64, 3 * 64);
		level2->gate_key_x = 9 * 64; level2->gate_key_y = 3 * 64;
		level2->gate->img = spr_stairs_up;
		level2->gate_x = 3 * 64; level2->gate_y = 31 * 64;
		

		// Level 2 enemies
			Sentry* lvl2_sentry = new Sentry();
			lvl2_sentry->CopyValues(sentry);
			lvl2_sentry->Move(3 * 64, 9 * 64);
			vector<int*> lvl2_path; lvl2_path.reserve(2);
			int lvl2_s1_p1[2] = { 3, 9 },
				lvl2_s1_p2[2] = { 10, 9 };
			lvl2_path.push_back(lvl2_s1_p1); lvl2_path.push_back(lvl2_s1_p2);
			lvl2_sentry->SetPath(lvl2_path);

			// sentry 2
			Sentry* lvl2_sentry2 = new Sentry();
			lvl2_sentry2->CopyValues(sentry);
			lvl2_sentry2->Move(8 * 64, 33 * 64);
			vector<int*> lvl2_path2; lvl2_path2.reserve(3);
			int	lvl2_s2_p1[2] = { 8, 33 },
				lvl2_s2_p2[2] = { 8, 38 },
				lvl2_s2_p3[2] = { 4, 38 };
			lvl2_path2.push_back(lvl2_s2_p1);
			lvl2_path2.push_back(lvl2_s2_p2);
			lvl2_path2.push_back(lvl2_s2_p3);
			lvl2_sentry2->SetPath(lvl2_path2);
			lvl2_sentry2->SetPath(lvl2_path2);

			// scorpion 1
			Sentry* lvl2_scorp = new Sentry();
			lvl2_scorp->CopyValues(scorpion);
			lvl2_scorp->Move(4 * 64, 5 * 64);
			vector<int*> lvl2_path3; lvl2_path3.reserve(3);
			int	lvl2_sc_p1[2] = { 4, 5 },
				lvl2_sc_p2[2] = { 7, 3 },
				lvl2_sc_p3[2] = { 10, 3 };
			lvl2_path3.push_back(lvl2_sc_p1);
			lvl2_path3.push_back(lvl2_sc_p2);
			lvl2_path3.push_back(lvl2_sc_p3);
			lvl2_scorp->SetPath(lvl2_path3);
			lvl2_scorp->SetPath(lvl2_path3);

			// scorpion 2
			Sentry* lvl2_scorp2 = new Sentry();
			lvl2_scorp2->CopyValues(scorpion);
			lvl2_scorp2->Move(8 * 64, 16 * 64);
			vector<int*> lvl2_path4; lvl2_path4.reserve(2);
			int	lvl2_sc2_p1[2] = { 8, 16 },
				lvl2_sc2_p2[2] = { 8, 21 };
			lvl2_path4.push_back(lvl2_sc2_p1);
			lvl2_path4.push_back(lvl2_sc2_p2);
			lvl2_scorp2->SetPath(lvl2_path4);
			lvl2_scorp2->SetPath(lvl2_path4);

			// scorpion 3
			Sentry* lvl2_scorp3 = new Sentry();
			lvl2_scorp3->CopyValues(scorpion);
			lvl2_scorp3->Move(3 * 64, 42 * 64);
			vector<int*> lvl2_path5; lvl2_path5.reserve(2);
			int	lvl2_sc3_p1[2] = { 3, 42 },
				lvl2_sc3_p2[2] = { 10, 42 };
			lvl2_path5.push_back(lvl2_sc3_p1);
			lvl2_path5.push_back(lvl2_sc3_p2);
			lvl2_scorp3->SetPath(lvl2_path5);
			lvl2_scorp3->SetPath(lvl2_path5);

			// add all enemies to level 2
			level2->enemies.push_back(lvl2_sentry);
			level2->enemies.push_back(lvl2_sentry2);
			level2->enemies.push_back(lvl2_scorp);
			level2->enemies.push_back(lvl2_scorp2);
			level2->enemies.push_back(lvl2_scorp3);
	
	//LEVEL 3
		Level* level3 = new Level();
		int level3_int_bg[60][13] = {
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },

			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },

			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 },
			{ 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3 }
		};
		int level3_int_deco[60][13] = {
			{ 0, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 3, 3, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 3, 3, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 3, 3, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 3, 3, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 3, 3, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 3, 3, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },

			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 3, 3, 3, 2, 0 },
			{ 0, 1, 3, 3, 3, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 3, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 3, 3, 3, 3, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 6, 6, 6, 6, 6, 6, 0, 0, 0, 2, 0 },

			{ 0, 1, 0, 0, 0, 0, 6, 6, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 6, 6, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 4, 4, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 4, 4, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 4, 4, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 4, 4, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 4, 4, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 4, 4, 4, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 4, 4, 4, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 0, 0, 4, 4, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 4, 0, 0, 4, 4, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 },
			{ 0, 1, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 0 }
		};
		// Assign the background and decorations for Level 3
		vector<vector<int>> vector_lvl3_bg, vector_lvl3_deco;
		for (int i = 0; i < 60; ++i) { vector_lvl3_bg.push_back(vector<int>(begin(level3_int_bg[i]), end(level3_int_bg[i]))); }
		for (int i = 0; i < 60; ++i) { vector_lvl3_deco.push_back(vector<int>(begin(level3_int_deco[i]), end(level3_int_deco[i]))); }
		Background* level3_bg = new Background(13, 60); level3_bg->SetLevel(vector_lvl3_bg);
		Background* level3_deco = new Background(13, 60); level3_deco->SetLevel(vector_lvl3_deco);
		level3_bg->SetTileSet(textures);
		level3_deco->SetTileSet(decorations);
		level3->background = level3_bg;
		level3->decorations = level3_deco;

		// Level 3 gate (NO KEY)
		level3->gate_key->img = 0;
		level3->gate_key->Move(-64, -64);
		level3->gate_key_x = -64; level3->gate_key_y = -64;
		level3->gate->img = spr_chair;
		level3->gate_x = 5 * 64; level3->gate_y = 1 * 64;

		// Level 3 enemies
			// scorp 1
			Sentry* lvl3_scorp = new Sentry();
			lvl3_scorp->CopyValues(scorpion);
			lvl3_scorp->Move(5 * 64, 37 * 64);
			vector<int*> lvl3_path; lvl3_path.reserve(2);
			int	lvl3_sc_p1[2] = { 5, 37 },
				lvl3_sc_p2[2] = { 5, 43 };
			lvl3_path.push_back(lvl3_sc_p1);
			lvl3_path.push_back(lvl3_sc_p2);
			lvl3_scorp->SetPath(lvl3_path);
			lvl3_scorp->SetPath(lvl3_path);

			// scorp 2
			Sentry* lvl3_scorp2 = new Sentry();
			lvl3_scorp2->CopyValues(scorpion);
			lvl3_scorp2->Move(2 * 64, 12 * 64);
			vector<int*> lvl3_path2; lvl3_path2.reserve(3);
			int	lvl3_sc2_p1[2] = { 2, 12 },
				lvl3_sc2_p2[2] = { 10, 12 },
				lvl3_sc2_p3[2] = { 10, 9 };
			lvl3_path2.push_back(lvl3_sc2_p1);
			lvl3_path2.push_back(lvl3_sc2_p2);
			lvl3_path2.push_back(lvl3_sc2_p3);
			lvl3_scorp2->SetPath(lvl3_path2);
			lvl3_scorp2->SetPath(lvl3_path2);

			// scorp 3
			Sentry* lvl3_scorp3 = new Sentry();
			lvl3_scorp3->CopyValues(scorpion);
			lvl3_scorp3->Move(9 * 64, 3 * 64);
			vector<int*> lvl3_path6; lvl3_path6.reserve(3);
			int	lvl3_sc3_p1[2] = { 8, 4 },
				lvl3_sc3_p2[2] = { 10, 2 };
			lvl3_path6.push_back(lvl3_sc3_p1);
			lvl3_path6.push_back(lvl3_sc3_p2);
			lvl3_scorp3->SetPath(lvl3_path6);
			lvl3_scorp3->SetPath(lvl3_path6);

			// sentry
			Sentry* lvl3_sentry = new Sentry();
			lvl3_sentry->CopyValues(sentry);
			lvl3_sentry->Move(3 * 64, 28 * 64);
			vector<int*> lvl3_path3; lvl3_path3.reserve(3);
			int	lvl3_s_p1[2] = { 3, 28 },
				lvl3_s_p2[2] = { 9, 28 },
				lvl3_s_p3[2] = { 9, 34 };
			lvl3_path3.push_back(lvl3_s_p1);
			lvl3_path3.push_back(lvl3_s_p2);
			lvl3_path3.push_back(lvl3_s_p3);
			lvl3_sentry->SetPath(lvl3_path3);
			lvl3_sentry->SetPath(lvl3_path3);

			// ranger
			Sentry* lvl3_ranger = new Sentry();
			lvl3_ranger->CopyValues(ranger);
			lvl3_ranger->Move(9 * 64, 20 * 64);
			vector<int*> lvl3_path4; lvl3_path4.reserve(3);
			int	lvl3_r_p1[2] = { 9, 20 },
				lvl3_r_p2[2] = { 5, 20 },
				lvl3_r_p3[2] = { 5, 24 };
			lvl3_path4.push_back(lvl3_r_p1);
			lvl3_path4.push_back(lvl3_r_p2);
			lvl3_path4.push_back(lvl3_r_p3);
			lvl3_ranger->SetPath(lvl3_path4);
			lvl3_ranger->SetPath(lvl3_path4);

			// ranger 2
			Sentry* lvl3_ranger2 = new Sentry();
			lvl3_ranger2->CopyValues(ranger);
			lvl3_ranger2->Move(8 * 64, 6 * 64);
			vector<int*> lvl3_path5; lvl3_path5.reserve(3);
			int	lvl3_r2_p1[2] = { 8, 7 },
				lvl3_r2_p2[2] = { 3, 7 };
			lvl3_path5.push_back(lvl3_r2_p1);
			lvl3_path5.push_back(lvl3_r2_p2);
			lvl3_ranger2->SetPath(lvl3_path5);
			lvl3_ranger2->SetPath(lvl3_path5);

			level3->enemies.push_back(lvl3_scorp);
			level3->enemies.push_back(lvl3_scorp2);
			level3->enemies.push_back(lvl3_scorp3);
			level3->enemies.push_back(lvl3_sentry);
			level3->enemies.push_back(lvl3_ranger);
			level3->enemies.push_back(lvl3_ranger2);
			
	// add levels to a vector
		vector<Level*> levels;
		levels.push_back(level1);
		levels.push_back(level2);
		levels.push_back(level3);

	// Create the player
		Player* player = new Player(128, 0, 64, 64, 2000, 350, 9);
		player->punch->img = spr_punch;
		player->AddAnimation(anim_player_up);
		player->AddAnimation(anim_player_down);
		player->AddAnimation(anim_player_left);
		player->AddAnimation(anim_player_right);
		player->AddAnimation(anim_player_dead);
		player->AddAnimation(anim_player_hit);
		player->AddAnimation(anim_player_punch_up);
		player->AddAnimation(anim_player_punch_down);
		player->AddAnimation(anim_player_punch_left);
		player->AddAnimation(anim_player_punch_right);
	
	// Add all entites to physics
	Physics* physics = new Physics();
	
	// Create the camera
	Camera* camera = new Camera(0, 0, 14, 11, 300);

	// Temp Variables for gameplay, filters, gameover
		// UI variables
			bool crawl = true, instr = true; float crawl_height = 700;
			float crawl_speed = 65.0;
			float idleuptime = 0.0, max_idle = 3.0;
			bool title_on = 1;
			float camTime = 0.0;
			int filter_y = -600, filter_y2 = 0;
		// scene variables
			int current_level = 0;
			bool setScene = true;
			bool gameover = false;
			bool gamewin = false;
			bool gameendsound = false;
		// input variables
			bool fullscreen = false;
			bool prevFramePunch = false;
			bool prevFrameSpace = false;
			int camInput[2] = { 0, 0 };
			player->input[0] = 0; player->input[1] = 0;
			int camMinY = 150, camMaxY = 350;
			int camera_player_pos = 0;	

	// Initialize deltaTime
	Uint64 NOW = SDL_GetTicks(), LAST = 0;
	float deltaTime;
	FMOD_RESULT result;
	FMOD_System_PlaySound(fmod, stream_titletheme, 0, false, &titleChan);
	FMOD_Sound_SetMode(stream_titletheme, FMOD_LOOP_NORMAL);
	FMOD_Sound_SetMode(stream_maintheme, FMOD_LOOP_NORMAL);
	// The game loop.
	kbState = SDL_GetKeyboardState(NULL);
	while (!shouldExit) {
		FMOD_System_Update(fmod); // update fmod
		FMOD_Channel_SetLoopCount(titleChan, -1);
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
		// Calculate deltatime
		NOW = SDL_GetTicks();
		deltaTime = (NOW - LAST) / 1000.0;
		LAST = NOW;
		// Player input
		if (kbState[SDL_SCANCODE_ESCAPE]) {
			shouldExit = 1;
		}
		// Fullscreen
		if (kbState[SDL_SCANCODE_F10]) {
			if (fullscreen) {
				SDL_SetWindowFullscreen(window, 0);
				fullscreen = false;
			}
			else {
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
				fullscreen = true;
			}
		}

		if (crawl) {
			if (instr) {
				if (kbState[SDL_SCANCODE_SPACE]) {
					instr = false;
					prevFrameSpace = true;
				}
				
				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT);
				// draw the instr
				glDrawSprite(spr_instructions, 0, 0, 800, 600);
			}
			else {
				if (kbState[SDL_SCANCODE_F10]) {
					if (fullscreen) {
						SDL_SetWindowFullscreen(window, 0);
						fullscreen = false;
					}
					else {
						SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
						fullscreen = true;
					}
				}
				if (kbState[SDL_SCANCODE_SPACE]) {
					if (!prevFrameSpace) {
						prevFrameSpace = true;
						crawl = false;
					}
				}
				else {
					prevFrameSpace = false;
				}

				// add to the crawl location
				crawl_height -= crawl_speed * deltaTime;
				if (crawl_height <= -800) {
					crawl = false;
				}

				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT);
				// draw the crawl
				glDrawSprite(spr_crawl_bg, 0, 0, 800, 600);
				glDrawSprite(spr_crawl_text, 0, crawl_height, 800, 800);
			}
		} else {
			if (kbState[SDL_SCANCODE_SPACE]) {
				if (!prevFrameSpace) {
					prevFrameSpace = true;
					if (!gameover) {
						if (title_on) {
							title_on = false;
							player->health = 20;
							((Actor*)player)->Move(128, 0);
							FMOD_Channel_Stop(titleChan);
							FMOD_Channel_SetLoopCount(titleChan, -1);
							FMOD_System_PlaySound(fmod, stream_maintheme, 0, false, &mainChan);
							gameendsound = false;
						}
					} else if (gameover || gamewin) {
						title_on = true;
						gameover = false;
						setScene = true;
						gamewin = false;
						current_level = 0;
						((Actor*)player)->Move(128, 200);
						camera->Move(0, 0);
						player->health = 20;
						FMOD_Channel_Stop(mainChan);
						FMOD_Channel_SetLoopCount(titleChan, -1);
						FMOD_System_PlaySound(fmod, stream_titletheme, 0, false, &titleChan);
						gameendsound = false;
					}
				}
			}
			else {
				prevFrameSpace = false;
			}
			// Set the scene if there is a level transition or reset
			if (setScene && !gamewin) {
				player->health = 20;
				camera->ClearScreen();
				physics->ResetComponents();
				currentLevel->background = levels[current_level]->background;
				currentLevel->decorations = levels[current_level]->decorations;
				currentLevel->enemies = levels[current_level]->enemies;
				currentLevel->gate->collision = false; currentLevel->gate_key->collision = false;
				currentLevel->gate_key->img = levels[current_level]->gate_key->img;
				currentLevel->gate_key_x = levels[current_level]->gate_key->x;
				currentLevel->gate_key_y = levels[current_level]->gate_key->y;
				currentLevel->gate->img = levels[current_level]->gate->img;
				currentLevel->gate_x = levels[current_level]->gate_x;
				currentLevel->gate_y = levels[current_level]->gate_y;
				currentLevel->Reset();
				if (current_level == levels.size() - 1) {
					player->hasKey = true;
					currentLevel->gate_key->collision = true;
					currentLevel->gate_key->Move(-64, -64);
				}
				// add the background and deco to cam and phsyics
				camera->SetBackground(currentLevel->background);
				camera->SetDecoration(currentLevel->decorations);
				physics->AddToPhysicsUpdate(currentLevel->decorations);
				// add the player to cam and phsyics
				camera->AddActor(player);
				camera->AddProjectile(player->punch);
				camera->AddProjectile(currentLevel->gate_key);
				camera->AddProjectile(currentLevel->gate);
				physics->AddToPhysicsUpdate(player);
				physics->AddToPhysicsUpdate(player->punch);
				physics->AddToPhysicsUpdate(currentLevel->gate_key);
				// add all enemies to cam and phsyics
				for (int i = 0; i < currentLevel->currentEnemies.size(); i++) {
					if (currentLevel->currentEnemies[i]->speed > 90)
						currentLevel->currentEnemies[i]->peanut->img = spr_peanut;
					else {
						currentLevel->currentEnemies[i]->peanut->img = spr_peanut_bag;
					}
					camera->AddActor(currentLevel->currentEnemies[i]);
					camera->AddProjectile(currentLevel->currentEnemies[i]->peanut);
					physics->AddToPhysicsUpdate(currentLevel->currentEnemies[i]);
					physics->AddToPhysicsUpdate(currentLevel->currentEnemies[i]->peanut);
				}
				setScene = false;
			}
			if (!gamewin && !title_on) {
				// player input
				if (kbState[SDL_SCANCODE_UP] || kbState[SDL_SCANCODE_W]) {
					player->input[1] = -1;
				}
				else if (kbState[SDL_SCANCODE_DOWN] || kbState[SDL_SCANCODE_S]) {
					player->input[1] = 1;
				}
				else { player->input[1] = 0; }

				if (kbState[SDL_SCANCODE_LEFT] || kbState[SDL_SCANCODE_A]) {
					player->input[0] = -1;
				}
				else if (kbState[SDL_SCANCODE_RIGHT] || kbState[SDL_SCANCODE_D]) {
					player->input[0] = 1;
				}
				else { player->input[0] = 0; }

				camera_player_pos = player->y - camera->y;
				if (camera_player_pos <= camMinY) {
					camInput[1] = -1;
				}
				else if (camera_player_pos >= camMaxY) {
					camInput[1] = 1;
				}
				else { camInput[1] = 0; }
				camInput[0] = 0;

				if (kbState[SDL_SCANCODE_SPACE] || kbState[SDL_SCANCODE_F]) {
					if (!prevFramePunch) {
						prevFramePunch = true;
						player->Punch();
					}
				}
				else {
					prevFramePunch = false;
				}
			}
			
			if (player->hasKey) {
				currentLevel->OpenGate();
				player->hasKey = false;
			}
			if (currentLevel->IsGateOpen() && AABBIntersect(player->BoxCollider, currentLevel->gate->BoxCollider)) {
				current_level++;
				if (current_level == levels.size()) {
					gamewin = true; gameover = true;
				}
				else {
					setScene = true;
				}
			}
			if (player->health <= 0) gameover = true;

			// Update
			if (!gamewin) {
				for (int i = 0; i < currentLevel->currentEnemies.size(); i++) {
					currentLevel->currentEnemies[i]->Update(deltaTime);
					currentLevel->currentEnemies[i]->SetTarget(player->x, player->y); // update target pos
				}

				player->Update(deltaTime);
				// update physics
				physics->Update(deltaTime);
				physics->DetectCollisions();
			}
			if (!gameover) {
				// move the camera
				camera->Move(deltaTime, camInput);
			}
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			// Game drawing goes here.
			camera->Draw(deltaTime);
			if (gameover && !gamewin) {
				if (!gameendsound) {
					FMOD_Channel_Stop(titleChan); FMOD_Channel_Stop(mainChan);
					FMOD_System_PlaySound(fmod, sound_gameover, 0, false, &titleChan);
					gameendsound = true;
					FMOD_Channel_SetLoopCount(titleChan, 0);
				}
				glDrawSprite(spr_gameover, 338, 262, 124, 76);
				glDrawSprite(spr_tryagain, 218, 400, 364, 48);
			}
			if (title_on) {
				glDrawSprite(spr_title, 0, 0, 800, 600);
			}
			if (gamewin) {
				if (!gameendsound) {
					FMOD_Channel_Stop(titleChan); FMOD_Channel_Stop(mainChan);
					FMOD_System_PlaySound(fmod, sound_fanfare, 0, false, &titleChan);
					gameendsound = true;
				}
				glDrawSprite(spr_endscreen, 0, 0, 800, 600);
			}
			
		}

		// move filters
		filter_y += 300 * deltaTime;
		if (filter_y >= 600) filter_y = -600;
		filter_y2 += 305 * deltaTime;
		if (filter_y2 >= 600) filter_y2 = -600;

		// Draw camera filters (should put this somewhere else)
		camTime += deltaTime;
		if (camTime > 1.0 && camTime < 2.0 && !crawl && !title_on && !gamewin) { glDrawSprite(spr_rec, 338, 16, 124, 36); }
		else if (camTime >= 2.0) { camTime = 0.0; }

		glDrawSprite(filter_crt, 0, filter_y, 800, 600);
		glDrawSprite(filter_crt, 0, filter_y2, 800, 600);
		if (!title_on) glDrawSprite(filter_vignette, 0, 0, 800, 600);

        // Present the most recent frame.*/
        SDL_GL_SwapWindow(window);
    }
	// Quit
    SDL_Quit(); return 0;
}
