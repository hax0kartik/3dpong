/*
 * Most of the source code was written by Patrick (http://blog.16software.com/getting-your-feet-wet-in-sdl-part-2-the-pong-clone)
 * I just added a basic enemy AI and fine tuned it for 3ds.
 */

#include "pong.h"

paddleData lPaddle, rPaddle;
ballData pongBall;
SDL_Surface *main_screen = NULL;
TTF_Font *score_font = NULL;

int main(int argc, char **argv) {

	srand(time(NULL));
	romfsInit();
	
	consoleDebugInit(debugDevice_CONSOLE);
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
	}

	 main_screen = SDL_SetVideoMode(WIN_WIDTH, WIN_HEIGHT, BITS_PER_PIXEL, SDL_TOPSCR | SDL_CONSOLEBOTTOM);
	 printf("[STDOUT]Console Init\n");
	 fprintf(stderr, "[STDERR]Console Init\n");
     if (load_all_images() != 0) {
	  fprintf(stderr, "Unable to load all images.\n");
	  quit_the_game();
     }

     if (setup_score_font() != 0) {
	  fprintf(stderr, "Unable to load font!\n");
	  quit_the_game();
     }

     set_initial_coordinates_for_images();
		
	 printf("----------------------------------------");
	 printf("\t\t\t\t\tCONTROLS:- \n\t\t\tPress START to begin\nUse DPAD Up/Down keys to contol the left Paddle\n");
	 printf("----------------------------------------");
     run_game_loop();
     
     quit_the_game();
     return 0;
}

int load_all_images(void) {

     SDL_Surface *paddle = image_load_from_file("romfs:/images/paddle.bmp", TRUE);
     if (paddle == NULL) {
	  return -1;
     }

     lPaddle.surface = paddle;
     rPaddle.surface = paddle;

     SDL_Surface *ball = image_load_from_file("romfs:/images/ball.bmp", TRUE);
     if (ball == NULL) {
	  return -1;
     }
     pongBall.surface = ball;

     return 0;
}

SDL_Surface *image_load_from_file(const char *path, int apply_colorkey) {

     SDL_Surface *loaded = SDL_LoadBMP(path);
     if (loaded == NULL) {
	  return NULL;
     }

     if (apply_colorkey == TRUE) {

	  SDL_SetColorKey(loaded, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			  (Uint32)SDL_MapRGB(loaded->format, 0, 0, 0));
     }

     SDL_Surface *optimized = SDL_DisplayFormat(loaded);
     SDL_FreeSurface(loaded);

     return optimized;
}

int setup_score_font(void) {

     if (TTF_Init() != 0) {
	  return -1;
     }

     score_font = TTF_OpenFont("romfs:/fonts/SAVEDBYZ.TTF", 32);
     if (score_font == NULL) {
	  return -1;
     }

     return 0;
}

void set_initial_coordinates_for_images(void) {

     lPaddle.x = 0;
     lPaddle.y = rand_between(0, WIN_HEIGHT - lPaddle.surface->h);

     rPaddle.x = WIN_WIDTH - rPaddle.surface->w;
     rPaddle.y = rand_between(0, WIN_HEIGHT - rPaddle.surface->h);

     pong_ball_reset();
}

void run_game_loop(void) {

     Uint32 start_ticks = 0, end_ticks = 0;
     SDL_Event event;
     Uint8 *keystate = NULL;
     
     for (;;) {

	  start_ticks = SDL_GetTicks();

	  while (SDL_PollEvent(&event)) {

	       switch (event.type) {

	       case SDL_QUIT:
		    quit_the_game();
	       }
	  }

	  keystate = SDL_GetKeyState(NULL);

	  handle_keystate(keystate);
	  redraw_all_images();

          pong_ball_handle_collision();
	   
	  end_ticks = SDL_GetTicks();
	  cap_fps(start_ticks, end_ticks);
     }
}

void handle_keystate(Uint8 *keystate) {

     if (keystate[RPADDLE_UP_KEY]) {
	  paddle_move(&lPaddle, DIRECTION_UP);
     }

     if (keystate[RPADDLE_DOWN_KEY]) {
	  paddle_move(&lPaddle, DIRECTION_DOWN);
     }

	 if (keystate[START_GAME_KEY]) {
		 if(pongBall.moving == FALSE)
	       pong_ball_set_in_motion();
     }
	 
     if (pongBall.moving == TRUE) {
	  pong_ball_move();
	  enemy_move(&rPaddle);
     }

     if (keystate[QUIT_GAME_KEY]) {
	  quit_the_game();
     }
}

void paddle_move(paddleData *paddle, unsigned int direction) {

     int difference = 
	  image_get_acceptable_y_movement(paddle->surface, paddle->y,
					  PADDLE_SPEED, direction);

     paddle->y += difference;
}

void enemy_move(paddleData *paddle)
{
	if(pongBall.x < main_screen->w / 2) return;
	unsigned int direction = 0;
	if(pongBall.y >= paddle->y) direction = DIRECTION_DOWN;
	else direction = DIRECTION_UP;
	int difference = image_get_acceptable_y_movement(paddle->surface, paddle->y, BALL_SPEED - 3, direction); //We'll keep AI at a disadvantage. This is important
	paddle->y += difference;
}

int image_get_acceptable_y_movement(SDL_Surface *surface, int y, int speed, unsigned int direction) {

     int difference = 0;
     
     if (direction & DIRECTION_UP) {
	
	  if (y - speed < 0) {
	       difference = -y;
	  } else {
	       difference = -speed;
	  }

     } else if (direction & DIRECTION_DOWN) {

	  if (y + speed + surface->h > main_screen->h) {
	       difference = (main_screen->h - y) - surface->h;
	  } else {
	       difference = +speed;
	  }
     }

     return difference;
}

void pong_ball_set_in_motion(void) {

     pongBall.moving = TRUE;
     pongBall.direction = 0;

     pongBall.direction |= (rand() % 2 == 0) ? DIRECTION_UP : DIRECTION_DOWN;
     pongBall.direction |= (rand() % 2 == 0) ? DIRECTION_LEFT : DIRECTION_RIGHT;
}

void pong_ball_move(void) {

     int difference = 
	  image_get_acceptable_y_movement(pongBall.surface, pongBall.y,
					  BALL_SPEED, pongBall.direction);

     pongBall.y += difference;

     if (abs(difference) != BALL_SPEED) {
	  if (pongBall.direction & DIRECTION_UP) {
	       pongBall.direction &= ~DIRECTION_UP;
	       pongBall.direction |= DIRECTION_DOWN;
	  } else if (pongBall.direction & DIRECTION_DOWN) {
	       pongBall.direction &= ~DIRECTION_DOWN;
	       pongBall.direction |= DIRECTION_UP;
	  }
     }

     if (pongBall.direction & DIRECTION_LEFT) {
	  pongBall.x -= BALL_SPEED;
     } else if (pongBall.direction & DIRECTION_RIGHT) {
	  pongBall.x += BALL_SPEED;
     }

     pongBall.hit_paddle = FALSE;
     pongBall.out_of_bounds = FALSE;

     if (pongBall.direction & DIRECTION_LEFT) {

	  if (pongBall.x < (lPaddle.x + lPaddle.surface->w)) {

	       if (pongBall.y + pongBall.surface->h < lPaddle.y ||
		   pongBall.y > lPaddle.y + lPaddle.surface->h) {

		    pongBall.out_of_bounds = TRUE;
			printf("Ball moved out of bounds.\nPress START to continue.\n");
	       } else {

		    pongBall.hit_paddle = TRUE;
		    pongBall.x = lPaddle.x + lPaddle.surface->w;
	       }
	  }
     } else if (pongBall.direction & DIRECTION_RIGHT) {

	  if (pongBall.x + pongBall.surface->w > rPaddle.x) {

	       if (pongBall.y + pongBall.surface->h < rPaddle.y ||
		   pongBall.y > rPaddle.y + rPaddle.surface->h) {

		    pongBall.out_of_bounds = TRUE;
			printf("Ball moved out of bounds.\nPress START to continue.\n");
	       } else {
		    pongBall.hit_paddle = TRUE;
		    pongBall.x = rPaddle.x - pongBall.surface->w;
	       }
	  }
     }
}

void redraw_all_images(void) {
     
     clear_screen();

     blit_surface(lPaddle.surface, main_screen, lPaddle.x, lPaddle.y);
     blit_surface(rPaddle.surface, main_screen, rPaddle.x, rPaddle.y);
     blit_surface(pongBall.surface, main_screen, pongBall.x, pongBall.y);
     draw_scores(lPaddle.score, rPaddle.score);

     SDL_UpdateRect(main_screen, 0, 0, 0, 0);
}

void clear_screen(void) {

     SDL_FillRect(main_screen, NULL, SDL_MapRGB(main_screen->format, 0, 0, 0));
}

int blit_surface(SDL_Surface *src, SDL_Surface *dest, Sint16 x, Sint16 y) {

     SDL_Rect offset;
     offset.x = x;
     offset.y = y;

     return SDL_BlitSurface(src, NULL, dest, &offset); 
}

void draw_scores(int left_score, int right_score) {

     char lscore_text[10], rscore_text[10];
     snprintf(lscore_text, sizeof(lscore_text), "%d", left_score);
     snprintf(rscore_text, sizeof(rscore_text), "%d", right_score);

     SDL_Surface *lscore_surface = render_text(lscore_text);
     SDL_Surface *rscore_surface = render_text(rscore_text);

     blit_surface(lscore_surface, main_screen, 
		  (WIN_WIDTH / 2) / 2 - lscore_surface->w / 2, 0);

     blit_surface(rscore_surface, main_screen,
		  ((WIN_WIDTH/2)/2) + (WIN_WIDTH/2) -
		  (rscore_surface->w / 2), 0);

     SDL_FreeSurface(lscore_surface);
     SDL_FreeSurface(rscore_surface);
}

SDL_Surface *render_text(const char *text) {

     SDL_Color color;
     color.r = 176; color.g = 219; color.b = 255;
     return TTF_RenderText_Solid(score_font, text, color);
}

void pong_ball_handle_collision(void) {

     if (pongBall.hit_paddle == TRUE) {

	  pong_ball_move_in_opposite_direction();
	  pongBall.hit_paddle = FALSE;

     } else if (pongBall.out_of_bounds == TRUE) {

	  if (pongBall.direction & DIRECTION_LEFT) {
	       rPaddle.score += 1;
	  } else if (pongBall.direction & DIRECTION_RIGHT) {
	       lPaddle.score += 1;
	  }

	  pong_ball_reset();
	  pongBall.out_of_bounds = FALSE;
     }
}

void pong_ball_move_in_opposite_direction(void) {

     unsigned int direction = (rand() % 2 == 0) ? DIRECTION_UP : DIRECTION_DOWN;
     
     if (pongBall.direction & DIRECTION_LEFT) {
	  direction |= DIRECTION_RIGHT;
     } else if (pongBall.direction & DIRECTION_RIGHT) {
	  direction |= DIRECTION_LEFT;
     }

     pongBall.direction = direction;
}

void pong_ball_reset(void) {

     pongBall.x = (WIN_WIDTH/2) - (pongBall.surface->w / 2);
     pongBall.y = rand_between(0, WIN_HEIGHT - pongBall.surface->h);
     pongBall.moving = FALSE;
}

void cap_fps(Uint32 start_ticks, Uint32 end_ticks) {
	
}

void quit_the_game(void) {

     SDL_FreeSurface(lPaddle.surface);      
     SDL_FreeSurface(pongBall.surface);
     
     if (TTF_WasInit()) {
          TTF_CloseFont(score_font);
	  TTF_Quit();
     }

     SDL_Quit();

     exit(0);
}