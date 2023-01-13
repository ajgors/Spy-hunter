#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<string.h>
#include "main.h"
#include <ctime>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#include <iostream>
#include <cstdlib>
#include "vector.h"
#include "cars_vector.h"
#include "grass_vector.h"
#include "bullet_vector.h"
#include <iostream>
#include <filesystem>

using namespace std;

struct car_t {
	int x = CAR_X;
	int y = CAR_Y;
	double speed = START_SPEED;
	int crashed_at = 0;
	int type = NORMAL;
	bool on_fire = false;
};

struct game_time_t {
	double world_time = 0;
	int t1 = 0;
	int t2 = 0;
};

struct fps_t {
	int fps = FPS;
	int desired_delta = 1000 / fps;
	int start_loop = 0;
};

struct grass_t {
	int y = 0;
	int width = 0;
};

struct bullet_t {
	int y = CAR_Y;
	int x = CAR_X;
	int fired_time = 0;
};

struct heart_t {
	int y = 0;
	int x = 0;
	bool is_active = false;
};

struct score_t {
	double points = 0;
	int halted_at = 0;
	int traveled_distance = 0;
};


struct final_score_t {
	double points = 0;
	double total_time = 0;
};

struct menus {
	bool running = true;
	bool pause = false;
	bool save_screen = false;
	bool list_view = true;
};

struct scores_t {
	final_score_t* scores = { 0 };
	int total_saves = 0;
	bool loaded = false;
};

struct game_t {
	vector_t grass_que = { 0 };
	car_vector_t cars = { 0 };
	grass_vector_t grass = { 0 };
	bullet_vector_t bullets = { 0 };
	int grass_width_on_car_y = 0;
	menus which_menu;
	score_t score = { 0 };
	int lives = START_LIVES;
};

struct textures_t {
	SDL_Texture* red_car = NULL;
	SDL_Texture* grass = NULL;
	SDL_Texture* blue_car = NULL;
	SDL_Texture* green_car = NULL;
	SDL_Texture* bullet = NULL;
	SDL_Texture* fire = NULL;
	SDL_Texture* heart = NULL;
};

struct colors_t {
	int czarny = 0;
	int niebieski = 0;
};


int main(int argc, char* argv[]) {
	srand(time(nullptr));
	SDL_Event event;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* screen, * charset = NULL;
	SDL_Texture* scrtex;
	textures_t textures;
	game_time_t time;
	car_t player_car;
	fps_t game_fps;
	game_t game;
	colors_t colors;
	heart_t heart;
	scores_t saved_scores;

	init_car_vector(&game.cars);
	init_grass_vector(&game.grass);
	init_vector(&game.grass_que);
	init_bullet_vector(&game.bullets);

	if (init(window, renderer, screen, scrtex)) {
		load_charset(charset);
		textures.red_car = load_texture("car_red.bmp", renderer);
		textures.grass = load_texture("grass.bmp", renderer);
		textures.blue_car = load_texture("car_blue.bmp", renderer);
		textures.green_car = load_texture("car_green.bmp", renderer);
		textures.bullet = load_texture("bullet.bmp", renderer);
		textures.fire = load_texture("fire.bmp", renderer);
		textures.heart = load_texture("heart.bmp", renderer);

		if (charset == NULL || textures.grass == NULL || textures.red_car == NULL || textures.blue_car == NULL
			|| textures.bullet == NULL || textures.fire == NULL || textures.heart == NULL) {
			free_textures(textures);
			free_memory(screen, scrtex, renderer, window);
			SDL_Quit();
		}
	};

	init_colors(colors, screen);
	generate_start_grass(game);

	while (game.which_menu.running) {

		if (game.which_menu.save_screen) {
			char saves[SAVES_NUMBER][128] = { 0 };
			load_saves(saves);
			show_saves_screen(screen, charset, scrtex, renderer, event, saves, colors);
			load_picked_save(event, game, time, player_car, saves);
		}
		else if (game.which_menu.pause) {
			show_pause_screen(screen, colors, charset, scrtex, renderer);
		}
		else if (game.which_menu.list_view) {
			show_list_screen(screen, colors, charset, scrtex, renderer, saved_scores, game);
		}
		else if (game.lives == 0) {
			show_gameover_screen(screen, colors, charset, scrtex, renderer, game);
		}
		else {
			game_fps.start_loop = SDL_GetTicks();
			SDL_RenderClear(renderer);
			generate_grass_que(game);
			calculate_time(time);
			generate_random_car(textures, &game, player_car);
			generate_random_heart(heart, time, game);
			pick_up_heart(game, player_car, heart);
			move_hostile_car_to_player(game, player_car);
			render_grass(game, renderer, textures.grass, player_car);
			render_implemented(screen, charset, scrtex, renderer, colors);
			check_for_grass_colision(player_car, game, time);
			render_fire(renderer, player_car, textures.fire);
			render_heart(renderer, heart, textures.heart, player_car);
			render_cars(renderer, textures, game, player_car);
			manage_cars_position(game, player_car, time);
			icrease_score(game, player_car);
			render_bullet(renderer, game, textures.bullet);
			render_legend(screen, charset, time, game_fps, renderer, scrtex, game, colors, player_car);
			scroll_grass(game, player_car);
			SDL_RenderPresent(renderer);
		}

		events_handling(event, player_car, game, time, saved_scores);
		cap_fps(game_fps, player_car);
	}

	free_textures(textures);
	free_memory(screen, scrtex, renderer, window);
	SDL_Quit();

	return 1;
}


//moves fired bullet 
void move_bullets(game_t& game) {

	for (int i = 0; i < game.bullets.count; i++) {
		game.bullets.ptr[i].y -= BULLET_SPEED;

		//remove bullet if it is out of screen
		if (game.bullets.ptr[i].y < 0) bullet_vector_delete(&game.bullets, i);
	}
}


//remove car if it is out of screen
void remove_cars_outside_screen(game_t& game) {

	for (int i = 0; i < game.cars.count; i++) {
		if (game.cars.ptr[i].y > SCREEN_HEIGHT + CAR_HEIGTH) {
			car_vector_delete(&game.cars, i);
		}
		else if (game.cars.ptr[i].y + LEGEND_HEIGHT + CAR_HEIGTH < 0) {
			car_vector_delete(&game.cars, i);
		}
	}
}


//render cars on screens
void render_cars(SDL_Renderer* renderer, textures_t& textures, game_t& game, car_t& player_car) {
	for (int i = 0; i < game.cars.count; i++) {
		if (game.cars.ptr[i].type == NORMAL) {
			render_car(game.cars.ptr[i], renderer, textures.blue_car);
		}
		else if (game.cars.ptr[i].type == HOSTILE) {
			render_car(game.cars.ptr[i], renderer, textures.green_car);
		}
	}
	if (!player_car.on_fire) {
		render_car(player_car, renderer, textures.red_car);
	}
}


void manage_cars_position(game_t& game, car_t& player_car, game_time_t& time) {

	for (int i = 0; i < game.cars.count; i++) {

		//move cars when road shrinks
		if (game.grass.ptr[0].width > game.grass.ptr[1].width) {
			if (game.cars.ptr[i].x < game.grass.ptr[0].width) game.cars.ptr[i].x += GRASS_WIDTH;
			else if (game.cars.ptr[i].x + CAR_WIDTH / 2 > SCREEN_WIDTH - game.grass.ptr[0].width) game.cars.ptr[i].x -= GRASS_WIDTH;
			if (game.grass.ptr[0].width == MIN_GRASS_WIDTH) game.cars.ptr[i].x -= GRASS_WIDTH;
		}

		game.cars.ptr[i].y += player_car.speed - game.cars.ptr[i].speed;

		//player bumping to other cars
		if (player_car.y - CAR_HEIGTH < game.cars.ptr[i].y && player_car.y > game.cars.ptr[i].y && player_car.x + CAR_WIDTH >= game.cars.ptr[i].x && player_car.x <= game.cars.ptr[i].x + CAR_WIDTH) {
			destroy_car(player_car, game, time);
			car_vector_delete(&game.cars, i);
		}

		//other cars bumping to each other
		for (int k = 0; k < game.cars.count; k++) {
			if (k == i) continue;
			if (game.cars.ptr[i].y - CAR_HEIGTH < game.cars.ptr[k].y && game.cars.ptr[i].y > game.cars.ptr[k].y && game.cars.ptr[i].x + CAR_WIDTH >= game.cars.ptr[k].x && game.cars.ptr[i].x <= game.cars.ptr[k].x + CAR_WIDTH) {
				game.cars.ptr[k].speed += SPEED_INCREMENT;
			}
		}

		//Check if bullet hit car (when hit removed from map)
		for (int k = 0; k < game.bullets.count; k++) {
			if (game.bullets.ptr[k].y - CAR_HEIGTH < game.cars.ptr[i].y && game.bullets.ptr[k].y > game.cars.ptr[i].y && game.bullets.ptr[k].x + CAR_WIDTH >= game.cars.ptr[i].x && game.bullets.ptr[k].x <= game.cars.ptr[i].x + CAR_WIDTH) {

				//save time when car wash shoted
				if (game.cars.ptr[i].type == NORMAL) {
					game.score.halted_at = SDL_GetTicks();
				}
				else if (game.cars.ptr[i].type == HOSTILE) {
					//add poits for shoting hostile car
					game.score.points += 100;
				}
				car_vector_delete(&game.cars, i);
				bullet_vector_delete(&game.bullets, k);
			}
		}
		
		remove_cars_outside_screen(game);
	}
}


void scroll_grass(game_t& game, car_t& player_car) {
	
	//update grass Y according to player speed
	for (int i = 0; i < game.grass.count; i++) {
		game.grass.ptr[i].y += player_car.speed;
	}

	//update traveled distance
	game.score.traveled_distance += player_car.speed;

	//remove grass outside of screen
	if (game.grass.ptr[game.grass.count - 1].y - GRASS_HEIGHT >= SCREEN_HEIGHT) {
		grass_pop_back(&game.grass);
	}

	//add new grass from que when last grass is almost outside of screen
	if (game.grass.ptr[game.grass.count - 1].y + GRASS_HEIGHT > SCREEN_HEIGHT && game.grass.count < NUMBER_OF_GRASS_TXT + 1) {
		grass_t grass;
		grass.y = game.grass.ptr[game.grass.count - 1].y + GRASS_HEIGHT - SCREEN_HEIGHT - GRASS_HEIGHT;
		grass.width = pop_back(&game.grass_que);

		grass_add_to_front(&game.grass, grass);
	}
}


//increase score when car traveled height of screen
void icrease_score(game_t& game, car_t& car) {

	//score is gained for every SCREEN HEIGHT pixels traveled
	if (game.score.traveled_distance >= SCREEN_HEIGHT) {
		game.score.traveled_distance -= SCREEN_HEIGHT;

		//stops score increase when scoring is halted
		if (SDL_GetTicks() - game.score.halted_at < HALT_TIME && game.score.halted_at > 0) return;
		game.score.halted_at = 0;
		game.score.points += 50;
	}
}


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screensprite
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
}


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}


SDL_Texture* load_texture(char s[], SDL_Renderer* renderer) {
	SDL_Surface* carSurface = SDL_LoadBMP(s);
	if (carSurface == nullptr) {
		cout << SDL_GetError();
		return NULL;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, carSurface);
	SDL_FreeSurface(carSurface);
	return texture;
}


//loads save names from file to array; returns number of saves
void load_saves(char saves[SAVES_NUMBER][128]) {
	char path[] = "./";
	char extension[] = ".bin";

	int i = 0;
	for (const auto& entry : filesystem::directory_iterator(path)) {
		if (entry.path().extension() == extension) {
			strcpy(saves[i], entry.path().stem().string().c_str());
			i++;
		}
	}
}


void show_saves_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[10][128], colors_t& colors) {
	SDL_RenderClear(renderer);
	SDL_FillRect(screen, NULL, colors.czarny);
	char text[128];
	sprintf(text, "Load game");
	DrawString(screen, TEXT_CENTER, SAVES_Y, text, charset);
	sprintf(text, " press number to load save");
	DrawString(screen, TEXT_CENTER, SAVES_Y + 20, text, charset);

	//prints all saves
	for (int i = 0; i < SAVES_NUMBER; i++) {
		if (saves[i][0] == '\0') break;
		_itoa(i + 1, text, 10);
		strcat_s(text, ") ");
		strcat_s(text, saves[i]);
		DrawString(screen, TEXT_CENTER, SAVES_Y + 30 + (20 * (i + 1)), text, charset);
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


void load_save(game_t& game, game_time_t& game_time, car_t& car, char file_name[]) {

	strcat(file_name, ".bin");

	FILE* file = fopen(file_name, "r");
	if (file == NULL) cout << "ERROR WHILE OPENING FILE";
	else {

		//load game struct

		//laod grass
		free(game.grass.ptr);
		init_grass_vector(&game.grass);
		int old_que_count = 0;
		fread(&old_que_count, sizeof(int), 1, file);

		grass_t saved_grass;
		for (int i = 0; i < old_que_count; i++) {
			fread(&saved_grass, sizeof(grass_t), 1, file);
			grass_push_back(&game.grass, saved_grass);
		}

		//load grass que
		free(game.grass_que.ptr);
		init_vector(&game.grass_que);
		old_que_count = 0;
		fread(&old_que_count, sizeof(game.grass_que.count), 1, file);

		int k = 0;
		for (int i = 0; i < old_que_count; i++) {
			fread(&k, sizeof(int), 1, file);
			push_back(&game.grass_que, k);
		}

		//load cars
		free(game.cars.ptr);
		init_car_vector(&game.cars);
		old_que_count = 0;
		fread(&old_que_count, sizeof(int), 1, file);

		car_t saved_car;
		for (int i = 0; i < old_que_count; i++) {
			fread(&saved_car, sizeof(car_t), 1, file);
			car_push_back(&game.cars, saved_car);
		}

		//load bullets
		free(game.bullets.ptr);
		init_bullet_vector(&game.bullets);
		old_que_count = 0;
		fread(&old_que_count, sizeof(int), 1, file);

		bullet_t saved_bullet;
		for (int i = 0; i < old_que_count; i++) {
			fread(&saved_bullet, sizeof(bullet_t), 1, file);
			bullet_push_back(&game.bullets, saved_bullet);
		}

		fread(&game.score, sizeof(game.score), 1, file);
		fread(&game.grass_width_on_car_y, sizeof(game.grass_width_on_car_y), 1, file);
		fread(&game.lives, sizeof(game.lives), 1, file);


		//load other structs
		fread(&car, sizeof(car), 1, file);
		fread(&game_time, sizeof(game_time), 1, file);
		game_time.t1 = SDL_GetTicks();
		fclose(file);
	}
}


void save_game(game_t& game, game_time_t& game_time, car_t& player_car) {
	
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[128];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 128, "%d-%m-%Y-%H-%M-%S", timeinfo);
	strcat(buffer, ".bin");

	FILE* file = fopen(buffer, "w");
	if (file == NULL) cout << "ERROR OPENING FILE" << endl;
	else {

		//save game struct

		//grass vector
		fwrite(&game.grass, sizeof(game.grass.count), 1, file);
		for (int i = 0; i < game.grass.count; i++) {
			fwrite(&game.grass.ptr[i], sizeof(game.grass.ptr[0]), 1, file);
		}

		//grass que vector
		fwrite(&game.grass_que.count, sizeof(game.grass_que.count), 1, file);
		for (int i = 0; i < game.grass_que.count; i++) {
			fwrite(&game.grass_que.ptr[i], sizeof(int), 1, file);
		}

		//cars vector
		fwrite(&game.cars.count, sizeof(game.cars.count), 1, file);
		for (int i = 0; i < game.cars.count; i++) {
			fwrite(&game.cars.ptr[i], sizeof(game.cars.ptr[0]), 1, file);
		}

		//bullets vector
		fwrite(&game.bullets.count, sizeof(game.bullets.count), 1, file);
		for (int i = 0; i < game.bullets.count; i++) {
			fwrite(&game.bullets.ptr[i], sizeof(game.bullets.ptr[0]), 1, file);
		}

		fwrite(&game.score, sizeof(game.score), 1, file);
		fwrite(&game.grass_width_on_car_y, sizeof(game.grass_width_on_car_y), 1, file);
		fwrite(&game.lives, sizeof(game.lives), 1, file);


		//save other stucts
		fwrite(&player_car, sizeof(player_car), 1, file);
		fwrite(&game_time, sizeof(game_time), 1, file);

		fclose(file);
	}
}


void pick_up_heart(game_t& game, car_t& player_car, heart_t& heart) {
	int left_heart, rigth_heart, top_heart, bottom_heart;
	int left_car, rigth_car, top_car, bottom_car;

	left_heart = heart.x;
	rigth_heart = heart.x + HEART_WIDTH;
	top_heart = heart.y;
	bottom_heart = heart.y + HEART_HEIGTH;

	left_car = player_car.x;
	rigth_car = player_car.x + CAR_WIDTH;
	top_car = player_car.y;
	bottom_car = player_car.y + CAR_HEIGTH;

	bool picked = true;

	//check if heart and car are in collision
	if (bottom_heart <= top_car) picked = false;
	if (top_heart >= bottom_car) picked = false;
	if (rigth_heart <= left_car) picked = false;
	if (left_heart >= rigth_car) picked = false;

	if (picked == true) {
		game.lives++;
		heart.y = 0;
		heart.x = 0;
		heart.is_active = false;
	}
}


int generate_random_x_on_road(game_t* game) {

	int road_width = SCREEN_WIDTH - 2 * game->grass.ptr[0].width;
	int x = (rand() & road_width) + 1 + game->grass.ptr[0].width;
	return x;
}


void generate_random_car(textures_t textures, game_t* game, car_t& player_car) {


	if (player_car.speed > 0) {
		if (game->cars.count <= MAX_CARS) {
			int n = (rand() % 300) + 1;
			if (n == 1) {
				car_t random_car;
				
				//generate X position of car
				int x = generate_random_x_on_road(game);
				if (x > SCREEN_WIDTH / 2) x -= CAR_WIDTH;
				random_car.x = x;
				random_car.y = 0 - CAR_HEIGTH / 2;


				//generate random car speed 
				// min-half player car speed 
				// max-player car speed
				
				int speed = 0;
				while (speed < player_car.speed / 2) {
					speed = (rand() % (int)player_car.speed-1) +1;
				}
				random_car.speed = speed;
				
				//generate type of car HOSITLE or NORMAL
				int type = rand() % 2;
				if (type == NORMAL) random_car.type = NORMAL;
				else random_car.type = HOSTILE;
		
				car_push_back(&game->cars, random_car);
			}
		}
	}
}

//generates heart when in unlimited lives stage
void generate_random_heart(heart_t& heart, game_time_t& time, game_t& game) {

	if (time.world_time <= INF_LIVES_TIME && !heart.is_active) {
		int n = (rand() % 100) + 1;
		if (n == 1) {
			int x = generate_random_x_on_road(&game);
			if (x > SCREEN_WIDTH / 2) x -= HEART_WIDTH;
			heart.x = x;
			heart.y = 0;
			heart.is_active = true;
		}
	}
}


//frees textures memory
void free_textures(textures_t& textures) {
	SDL_DestroyTexture(textures.red_car);
	SDL_DestroyTexture(textures.grass);
	SDL_DestroyTexture(textures.blue_car);
	SDL_DestroyTexture(textures.heart);
	SDL_DestroyTexture(textures.bullet);
	SDL_DestroyTexture(textures.fire);
}


//loads charset bitmap
void load_charset(SDL_Surface*& charset) {
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) cout << SDL_GetError();
	SDL_SetColorKey(charset, true, 0x000000);
}


//frees game allocated memory
void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}


//inits windows, renderer screen and screntexture
int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return ERROR;
	}

	int rc;
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return ERROR;
	};

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	return SUCCESS;
}


void respawn_car(car_t& player_car, game_t& game) {

	if (SDL_GetTicks() - player_car.crashed_at >= CAR_RESPAWN_TIME) {

		if (game.grass_width_on_car_y == MIN_GRASS_WIDTH) {
			
			int n = rand() % 2;
			if(n == 0) player_car.x = CAR_X + GRASS_WIDTH;
			else player_car.x = CAR_X - GRASS_WIDTH;
		}
		else {
			player_car.x = CAR_X;
		}

		player_car.y = CAR_Y;
		player_car.speed = START_SPEED;
		player_car.on_fire = false;
	}
}


void destroy_car(car_t& player_car, game_t& game, game_time_t& time) {

	player_car.speed = 0;

	if (player_car.on_fire == false) {
		player_car.crashed_at = SDL_GetTicks();
		if (time.world_time >= INF_LIVES_TIME) {
			game.lives -= 1;
		}
	}
	player_car.on_fire = true;
}


//check if car is in grass 
void check_for_grass_colision(car_t& player_car, game_t& game, game_time_t& time) {


	if (!player_car.on_fire) {
		bool car_on_left_grass = player_car.x + CAR_WIDTH/2 <= game.grass_width_on_car_y;
		bool car_on_right_grass = player_car.x + CAR_WIDTH/2 >= SCREEN_WIDTH - game.grass_width_on_car_y;
		bool car_on_middle_grass = player_car.x == SCREEN_WIDTH / 2 - GRASS_WIDTH / 2;

		if (car_on_left_grass || car_on_right_grass || (car_on_middle_grass && game.grass_width_on_car_y == MIN_GRASS_WIDTH)) {
			destroy_car(player_car, game, time);
		}
	}
	else {
		respawn_car(player_car, game);
	}
}


//render implemented rectangle containing implemented elements
void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, colors_t& colors) {
	char text[128];

	DrawRectangle(screen, SCREEN_WIDTH - IMPLEMENTED_WIDTH, SCREEN_HEIGHT - IMPLEMENTED_HEIGHT, IMPLEMENTED_WIDTH, IMPLEMENTED_HEIGHT, colors.czarny, colors.niebieski);
	sprintf(text, "a b c d e f g");
	DrawString(screen, screen->w - IMPLEMENTED_WIDTH, screen->h - IMPLEMENTED_HEIGHT, text, charset);
	sprintf(text, "h i j k m");
	DrawString(screen, screen->w - IMPLEMENTED_WIDTH, screen->h - (IMPLEMENTED_HEIGHT - 10), text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

	SDL_Rect roadRect_left;
	roadRect_left.x = SCREEN_WIDTH - IMPLEMENTED_WIDTH;
	roadRect_left.y = SCREEN_HEIGHT - IMPLEMENTED_HEIGHT;
	roadRect_left.w = IMPLEMENTED_WIDTH;
	roadRect_left.h = IMPLEMENTED_HEIGHT;

	SDL_RenderCopy(renderer, scrtex, &roadRect_left, &roadRect_left);
}


void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car) {

	for (int i = 0; i < game.grass.count; ++i) {

		SDL_Rect roadRect_left = { 0 };
		roadRect_left.x = 0;
		roadRect_left.y = game.grass.ptr[i].y;
		roadRect_left.w = game.grass.ptr[i].width;
		roadRect_left.h = GRASS_HEIGHT;

		SDL_Rect roadRect_right = { 0 };
		roadRect_right.x = SCREEN_WIDTH - game.grass.ptr[i].width;
		roadRect_right.y = game.grass.ptr[i].y;
		roadRect_right.w = game.grass.ptr[i].width;
		roadRect_right.h = GRASS_HEIGHT;

		//save grass width on car Y coordiante (used for car grass collision)
		if (roadRect_right.y <= CAR_Y + CAR_HEIGTH && roadRect_right.y >= CAR_Y && game.grass.ptr[i].width > 0) {
			game.grass_width_on_car_y = game.grass.ptr[i].width;
		}

		//render grass in middle when grass is minimal width (to make 2 roads)
		if (game.grass.ptr[i].width == MIN_GRASS_WIDTH) {
			SDL_Rect roadRect_middle = { 0 };
			roadRect_middle.x = SCREEN_WIDTH / 2 - GRASS_WIDTH / 2;
			roadRect_middle.y = game.grass.ptr[i].y;
			roadRect_middle.w = MIN_GRASS_WIDTH;
			roadRect_middle.h = GRASS_HEIGHT;
			SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_middle);
		}

		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_left);
		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_right);
	}
}


//generates starting grass width
void generate_start_grass(game_t& game) {

	if (game.grass.count == 0) {
		for (int i = 0; i < NUMBER_OF_GRASS_TXT; i++) {

			grass_t grass;
			grass.y = GRASS_HEIGHT * i;
			grass.width = 6 * GRASS_WIDTH;

			grass_push_back(&game.grass, grass);
		}
	}
}


// n = 1 - widen grass (narrow road)
// n = 2 - narrow grass (widen road)
// n = 3 - no changes
void generate_grass_que(game_t& game) {

	if (game.grass_que.count == 0) {
		int n = (rand() % 3) + 1;
		if (n == 1) {
			//if grass can't be wider generate max width grasss
			if (game.grass.ptr[0].width == MAX_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MAX_GRASS_WIDTH);
				}
			}
			//widen grass in 3 steps
			else {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
				{
					if (i < NUMBER_OF_GRASS_TXT && game.grass.ptr[0].width + 3 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0].width + 3 * GRASS_WIDTH);
					}
					else if (i >= NUMBER_OF_GRASS_TXT && i < 2 * NUMBER_OF_GRASS_TXT && game.grass.ptr[0].width + 2 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {

						push_back(&game.grass_que, game.grass.ptr[0].width + 2 * GRASS_WIDTH);
					}
					else if (i >= 2 * NUMBER_OF_GRASS_TXT && i < NUMBER_OF_GRASS_TXT * 3 && game.grass.ptr[0].width + 1 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0].width + 1 * GRASS_WIDTH);
					}
					else {
						push_back(&game.grass_que, MAX_GRASS_WIDTH);
					}
				}
			}
		}
		else if (n == 2) {
			//if grass can't be narrower generate min width grasss
			if (game.grass.ptr[0].width == MIN_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MIN_GRASS_WIDTH);
				}
			}
			//narrow grass in 3 steps
			else {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
				{
					if (i < NUMBER_OF_GRASS_TXT && game.grass.ptr[0].width - 3 * GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0].width - 3 * GRASS_WIDTH);
					}
					else if (i >= NUMBER_OF_GRASS_TXT && i < 2 * NUMBER_OF_GRASS_TXT && game.grass.ptr[0].width - 2 * GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0].width - 2 * GRASS_WIDTH);
					}
					else if (i >= 2 * NUMBER_OF_GRASS_TXT && i < NUMBER_OF_GRASS_TXT * 3 && game.grass.ptr[0].width - GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0].width - 1 * GRASS_WIDTH);
					}
					else {
						push_back(&game.grass_que, MIN_GRASS_WIDTH);
					}
				}
			}
		}
		//generate grass without changes
		else {
			for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
			{
				push_back(&game.grass_que, game.grass.ptr[0].width);
			}
		}
	}
}


//caps fps to desiried 
void cap_fps(fps_t& game_fps, car_t& car) {
	int delta = (SDL_GetTicks() - game_fps.start_loop);
	if (delta < game_fps.desired_delta) {
		SDL_Delay(game_fps.desired_delta - delta);
	}
}


//renders legend containins game informations
void render_legend(SDL_Surface* screen, SDL_Surface* charset, game_time_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game, colors_t& colors, car_t &player_car) {

	char text[128];

	DrawRectangle(screen, LEGEND_X, LEGEND_Y, SCREEN_WIDTH, LEGEND_HEIGHT, colors.czarny, colors.niebieski);
	sprintf(text, "Igor Stadnicki 193435");
	DrawString(screen, TEXT_CENTER, LEGEND_Y + 5, text, charset);

	if (time.world_time >= INF_LIVES_TIME) {
		sprintf(text, "Time = %.1lf s  score: %.0lf  lives: %d, car speed: %.1lf", time.world_time, game.score.points, game.lives, player_car.speed);
	}
	else {
		sprintf(text, "Time = %.1lf s  score: %.0lf  lives: unlimited for %ds (%d), car speed: %.1lf", time.world_time, game.score.points, INF_LIVES_TIME, game.lives, player_car.speed);
	}
	DrawString(screen, TEXT_CENTER, LEGEND_Y + 20, text, charset);

	sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie, n - nowa gra");
	DrawString(screen, TEXT_CENTER, LEGEND_Y + 35, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

	SDL_Rect roadRect_left;
	roadRect_left.x = LEGEND_X;
	roadRect_left.y = LEGEND_Y;
	roadRect_left.w = SCREEN_WIDTH;
	roadRect_left.h = LEGEND_HEIGHT;

	SDL_RenderCopy(renderer, scrtex, &roadRect_left, &roadRect_left);
}


//calculates game time
void calculate_time(game_time_t& time) {
	time.t2 = SDL_GetTicks();
	time.world_time += (time.t2 - time.t1) * 0.001;
	time.t1 = time.t2;
}


//fire a bullet (can fire every 0.5 second)
void fire_bullet(game_t& game, car_t& player_car) {

	if (!player_car.on_fire) {
		if (SDL_GetTicks() - game.bullets.ptr[game.bullets.count - 1].fired_time < 500 && game.bullets.count > 0) return;
		bullet_t bullet;
		bullet.fired_time = SDL_GetTicks();
		bullet.x = player_car.x + CAR_WIDTH / 2;
		bullet_push_back(&game.bullets, bullet);
	}

}


void sort_by_time(scores_t &saved_scores) {
	int size = saved_scores.total_saves;
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size - 1; ++j)
		{
			if (saved_scores.scores[j].total_time < saved_scores.scores[i].total_time) {
				final_score_t temp = saved_scores.scores[i];
				saved_scores.scores[i] = saved_scores.scores[j];
				saved_scores.scores[j] = temp;
			}
		}
	}
	
}


void sort_by_points(scores_t& saved_scores) {
	int size = saved_scores.total_saves;
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size - 1; ++j)
		{
			if (saved_scores.scores[j].points < saved_scores.scores[i].points) {
				final_score_t temp = saved_scores.scores[i];
				saved_scores.scores[i] = saved_scores.scores[j];
				saved_scores.scores[j] = temp;
			}
		}
	}
}


//handling input
void events_handling(SDL_Event& event, car_t& car, game_t& game, game_time_t& time, scores_t & saved_scores) {

	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LEFT] && !game.which_menu.pause && car.on_fire == false && car.speed > 0) {
		car.x -= CAR_MOVE_PIXELS;
	}
	if (state[SDL_SCANCODE_RIGHT] && !game.which_menu.pause && car.on_fire == false && car.speed > 0) {
		car.x += CAR_MOVE_PIXELS;
	}
	if (state[SDL_SCANCODE_UP] && !game.which_menu.pause && car.on_fire == false) {
		if (car.speed + SPEED_INCREMENT <= MAX_SPEED)
			car.speed += SPEED_INCREMENT;
	}
	if (state[SDL_SCANCODE_DOWN] && !game.which_menu.pause && car.on_fire == false) {
		if (car.speed - SPEED_INCREMENT >= 0)
			car.speed -= SPEED_INCREMENT;
	}

	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				game.which_menu.running = false;
			}
			else if (event.key.keysym.sym == SDLK_n) restart_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_i) game.which_menu.list_view = true;
			else if (event.key.keysym.sym == SDLK_t && game.which_menu.list_view) sort_by_time(saved_scores);
			else if (event.key.keysym.sym == SDLK_p && game.which_menu.list_view) sort_by_points(saved_scores);
			else if (event.key.keysym.sym == SDLK_p) stop_game(car, time, game);
			else if (event.key.keysym.sym == SDLK_s) save_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_l) game.which_menu.save_screen = true;
			else if (event.key.keysym.sym == SDLK_SPACE) fire_bullet(game, car);
			else if (event.key.keysym.sym == SDLK_a && game.lives == 0) {
				save_score(game, time, saved_scores);
				saved_scores.loaded = false;
				game.which_menu.list_view = true;
			}
			break;
		case SDL_QUIT:
			game.which_menu.running = false;
			break;
		}
	}
}


void stop_game(car_t& car, game_time_t& time, game_t& game) {
	game.which_menu.pause == false ? game.which_menu.pause = true : game.which_menu.pause = false;
	time.t1 = SDL_GetTicks();
}


void restart_game(game_t& game, game_time_t& time, car_t& car) {
	free(game.grass_que.ptr);
	init_vector(&game.grass_que);
	free(game.cars.ptr);
	init_car_vector(&game.cars);
	free(game.grass.ptr);
	init_grass_vector(&game.grass);
	generate_start_grass(game);
	free(game.bullets.ptr);
	init_bullet_vector(&game.bullets);
	game.which_menu.list_view = false;
	game.score = { 0 };
	game.lives = START_LIVES;
	time.world_time = 0;
	time.t1 = SDL_GetTicks();
	time.t2 = 0;
	car.speed = START_SPEED;
	car.x = CAR_X;
	car.y = CAR_Y;
}


//Car rendering
void render_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture) {
	SDL_Rect carRect;
	carRect.x = car.x;
	carRect.y = car.y;
	carRect.w = CAR_WIDTH;
	carRect.h = CAR_HEIGTH;
	SDL_RenderCopy(renderer, carTexture, nullptr, &carRect);
}


//rendres bullets
void render_bullet(SDL_Renderer* renderer, game_t& game, SDL_Texture* bullet_texture) {

	for (int i = 0; i < game.bullets.count; i++) {
		SDL_Rect bullet_rect = { game.bullets.ptr[i].x, game.bullets.ptr[i].y, BULLET_WIDTH, BULLET_HEIGTH };
		SDL_RenderCopy(renderer, bullet_texture, NULL, &bullet_rect);
	}
	move_bullets(game);
}


//renders fire
void render_fire(SDL_Renderer* renderer, car_t& player_car, SDL_Texture* fire) {

	if (player_car.on_fire) {
		SDL_Rect fire_rect = { player_car.x, player_car.y, FIRE_WIDTH, FIRE_HEIGTH };
		SDL_RenderCopy(renderer, fire, NULL, &fire_rect);
	}
}


//renders heart and moves it
void render_heart(SDL_Renderer* renderer, heart_t& heart, SDL_Texture* heart_txt, car_t& player_car) {

	if (heart.is_active) {
		SDL_Rect heart_rect = { heart.x, heart.y, HEART_WIDTH, HEART_HEIGTH };
		SDL_RenderCopy(renderer, heart_txt, NULL, &heart_rect);

		if (heart.y >= 0) {
			heart.y += player_car.speed;
		}
		if (heart.y >= SCREEN_HEIGHT) {
			heart.y = 0;
			heart.x = 0;
			heart.is_active = false;
		}
	}
}


//initializes colors in struct
void init_colors(colors_t& colors, SDL_Surface* screen) {
	colors.czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	colors.niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
}


//shows game over screen
void show_gameover_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, game_t& game) {
	SDL_FillRect(screen, NULL, colors.czarny);
	char text[128];
	sprintf(text, "GAME OVER SCORE: %.lf", game.score.points);
	DrawString(screen, TEXT_CENTER, 50, text, charset);
	sprintf(text, "Click n for new game");
	DrawString(screen, TEXT_CENTER, 66, text, charset);
	sprintf(text, "Click a to append score to list");
	DrawString(screen, TEXT_CENTER, 82, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


//shows pause screen
void show_pause_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
	SDL_FillRect(screen, NULL, colors.czarny);
	char text[128];
	sprintf(text, "GAME PAUSED");
	DrawString(screen, TEXT_CENTER, 50, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


//loads piceked by user save
void load_picked_save(SDL_Event& event, game_t& game, game_time_t& time, car_t& player_car, char  saves[10][128]) {
	int save_number = event.key.keysym.sym - '0' - 1;
	if (save_number >= 0 && save_number < SAVES_NUMBER) {
		load_save(game, time, player_car, saves[save_number]);
		game.which_menu.save_screen = false;
	}
}


//randomly move hostile car towards player car
void move_hostile_car_to_player(game_t& game, car_t& player_car) {
	if (!player_car.on_fire) {
		for (int i = 0; i < game.cars.count; i++) {
			if (game.grass.ptr[game.grass.count - 1].width == MIN_GRASS_WIDTH && game.cars.ptr[i].x < SCREEN_WIDTH / 2) continue;
			if (game.cars.ptr[i].type == HOSTILE) {
				if (game.cars.ptr[i].x > player_car.x) {
					game.cars.ptr[i].x -= CAR_MOVE_PIXELS;
				}
				else if (game.cars.ptr[i].x < player_car.x) {
					game.cars.ptr[i].x += CAR_MOVE_PIXELS;
				}
			}
		}
	}
}


int load_scores_size() {

	//get total number of saved scores
	FILE* file = fopen(SAVE_FILE, "r+");
	int size = 0;
	if (file == NULL) cout << "ERROR WHILE OPENING FILE";
	else {
		fscanf(file, "%d", &size);
		fclose(file);
	}
	return size;
}


void save_score(game_t& game, game_time_t& time, scores_t& saved_scores) {

	saved_scores.total_saves = load_scores_size() + 1;
	FILE* file = fopen(SAVE_FILE, "r+");
	if (file == NULL) cout << "ERROR WHILE OPENING FILE";
	else {
		fseek(file, 0, SEEK_SET);
		fprintf(file, "%d\n", saved_scores.total_saves);
		fseek(file, 0, SEEK_END);
		fprintf(file, "%.lf\n%.lf\n", game.score.points, time.world_time);
		fclose(file);
	}
}


void load_scores_list(scores_t& saved_scores) {

	FILE* file = fopen(SAVE_FILE, "r");

	//create file if does not exist
	if (file == NULL) {
		file = fopen(SAVE_FILE, "w");
		saved_scores.total_saves = 0;
		fprintf(file, "%d", saved_scores.total_saves);
		return;
	}

	file = fopen(SAVE_FILE, "r+");
	int size = load_scores_size();
	saved_scores.scores = (final_score_t*)malloc(size * sizeof(final_score_t));
	if (file == NULL) cout << "ERROR WHILE OPENING FILE";
	else {
		saved_scores.total_saves = size;
		fseek(file, 1, SEEK_SET);
		if (saved_scores.scores != NULL) {
			for (int i = 0; i < size; i++) {
				final_score_t final_score = { 0 };
				fscanf(file, "%lf", &final_score.points);
				fscanf(file, "%lf", &final_score.total_time);
				saved_scores.scores[i] = final_score;
			}
		}
		fclose(file);
	}
}


//shows pause screen
void show_list_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, scores_t& saved_scores, game_t& game) {
	SDL_FillRect(screen, NULL, colors.czarny);

	char text[128];
	if (saved_scores.loaded == false) {
		load_scores_list(saved_scores);
		saved_scores.loaded = true;
		sort_by_points(saved_scores);
	}

	sprintf(text, "LIST OF SCORES");
	DrawString(screen, TEXT_CENTER, 34, text, charset);
	sprintf(text, "sort by: time - t, points - p");
	DrawString(screen, TEXT_CENTER, 50, text, charset);
	sprintf(text, "Click n for new game");
	DrawString(screen, TEXT_CENTER, 66, text, charset);
	for (int i = 0; i < saved_scores.total_saves; i++) {
		sprintf(text, "%d) %.lf points in %.lf seconds", i + 1, saved_scores.scores[i].points, saved_scores.scores[i].total_time);
		DrawString(screen, TEXT_CENTER, 82 + 16 * (i + 1), text, charset);
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}