/****
 **** Check out the walk-through at:
 **** 
 **** http://blog.16software.com/getting-your-feet-wet-in-sdl-part-2-the-pong-clone
 ****/

#include <SDL/SDL.h>		/* All SDL_* functions. */
#include <SDL/SDL_ttf.h>	/* All TTF_* functions. */
#include <3ds.h>
#include <time.h>		/* So we can seed the random number generator to the current time. */

#define WIN_WIDTH 400
#define WIN_HEIGHT 240		
#define BITS_PER_PIXEL 32	
#define FRAMES_PER_SECOND 30	
#define TRUE 1			
#define FALSE 0
#define LPADDLE_UP_KEY SDLK_a	  	/* 'a' key moves left paddle UP */
#define LPADDLE_DOWN_KEY SDLK_b
#define RPADDLE_UP_KEY SDLK_UP	   	/* Up-arrow key moves right paddle UP */
#define RPADDLE_DOWN_KEY SDLK_DOWN 
#define QUIT_GAME_KEY SDLK_ESCAPE  	/* Escape quits the game. */
#define START_GAME_KEY SDLK_RETURN  	/* Space starts the game. */
#define PADDLE_SPEED 10		   	/* Number of pixels paddle moves per frame. */
#define BALL_SPEED 8	   	/* Number of pixels ball moves per frame */

/* Convenience macro to generate a random number between (min, max) */
#define rand_between(min, max) (rand() % ((max)-(min) + 1)) + (min)

enum {
     DIRECTION_UP = (1<<0),
     DIRECTION_DOWN = (1<<1),
     DIRECTION_LEFT = (1<<2),
     DIRECTION_RIGHT = (1<<3)
};

/* Structure definition to hold paddle-specific data. */
typedef struct paddleData {

     SDL_Surface *surface;	/* the actual image */
     int x, y;			/* X/Y coordinates of the image on screen. */
     int score;			/* the score for the paddle */
} paddleData;

/* Structure definition to hold ball-specific data. */
typedef struct ballData {

     SDL_Surface *surface;	/* the actual image */
     int x, y;			/* X/Y coordinates of the image on screen */
     int moving;		/* is the ball currently moving? */
     unsigned int direction;	/* what direction is the ball moving in? */
     int out_of_bounds;		/* is ball out of bounds (past the paddle) */
	 int restart;
     int hit_paddle;		/* did the ball hit the paddle? */
} ballData;

/* Function declarations; */
void run_game_loop(void);
void quit_the_game(void);
void handle_keystate(Uint8 *keystate);
void cap_fps(Uint32 start_ticks, Uint32 end_ticks);
void paddle_move(paddleData *paddle, unsigned int direction);
void clear_screen(void);
void redraw_all_images(void);
void set_initial_coordinates_for_images(void);
void pong_ball_reset(void);
void pong_ball_set_in_motion(void);
void pong_ball_move(void);
void pong_ball_handle_collision(void);
void pong_ball_move_in_opposite_direction(void);
void draw_scores(int left_score, int right_score);
int blit_surface(SDL_Surface *src, SDL_Surface *dest,
		 Sint16 x, Sint16 y);
int image_get_acceptable_y_movement(SDL_Surface *surface, int y,
				    int speed, unsigned int direction);
int setup_score_font(void);
int load_all_images(void);
SDL_Surface *image_load_from_file(const char *path, int apply_colorkey);
SDL_Surface *render_text(const char *text);
void enemy_move(paddleData *paddle);