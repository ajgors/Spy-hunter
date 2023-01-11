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
	int speed = START_SPEED;
	bool in_grass = false;
	int time = 0;
	int type = NORMAL;
};

struct gameTime_t {
	double world_time = SDL_GetTicks();
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
	int time = 0;
};

struct fire_t {
	int y = 0;
	int x = 0;
};

struct heart_t {
	int y = OUT_OF_SCREEN;
	int x = 0;
};

struct game_t {
	vector_t grass_que = { 0 };
	car_vector_t cars = { 0 };
	grass_vector_t grass = { 0 };
	bullet_vector_t bullets = { 0 };
	int grass_width_on_car_y = 0;
	bool running = true;
	bool pause = false;
	bool save_screen = false;
	double score = 0;
	int score_halt = 0;
	int traveled_distance = 0;
	int lives = START_LIVES;
	fire_t fire = { 0 };
	heart_t heart;
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
	gameTime_t time;
	car_t player_car;
	fps_t game_fps;
	game_t game;
	colors_t colors;

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
	generate_start_road(game);

	while (game.running) {

		if (game.save_screen) {
			char saves[SAVES_NUMBER][128] = { 0 };
			load_saves(saves);
			show_saves_screen(screen, charset, scrtex, renderer, event, saves);
			load_picked_save(event, game, time, player_car, saves);
		}
		else if (game.pause) {
			show_pause_screen(screen, colors, charset, scrtex, renderer);
		}
		else if (game.lives == 0) {
			show_gameover_screen(screen, colors, charset, scrtex, renderer, game);
		}
		else {
			game_fps.start_loop = SDL_GetTicks();
			SDL_RenderClear(renderer);
			generate_road_que(game);
			calculate_time(time);
			generate_random_car(textures, &game, player_car);
			generate_random_heart(game, time);
			pick_up_heart(game, player_car);
			render_grass(game, renderer, textures.grass, player_car);
			render_implemented(screen, charset, scrtex, renderer);
			check_for_grass_colision(player_car, game, time);
			render_fire(renderer, game, textures.fire);
			render_heart(renderer, game, textures.heart, player_car);
			update_cars_speed(game);
			render_cars(renderer, textures, game, player_car);
			manage_cars_position(game, player_car, time);
			icrease_score(game, player_car);
			render_bullet(renderer, game, textures.bullet);
			render_legend(screen, charset, time, game_fps, renderer, scrtex, game, colors);
			scroll_grass(game, player_car);
			SDL_RenderPresent(renderer);
		}

		events_handling(event, player_car, game, time);
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


//change speed of cars randomly
void update_cars_speed(game_t& game) {
	for (int i = 0; i < game.cars.count; i++) {
		int n = (rand() % 1000) + 1;

		if (n == 1 && game.cars.ptr[i].speed + SPEED_INCREMENT < MAX_SPEED) game.cars.ptr[i].speed += SPEED_INCREMENT;
		if (n == 1000 && game.cars.ptr[i].speed - SPEED_INCREMENT > 0) game.cars.ptr[i].speed -= SPEED_INCREMENT;
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
	render_car(player_car, renderer, textures.red_car);
}


void manage_cars_position(game_t& game, car_t& player_car, gameTime_t& time) {

	for (int i = 0; i < game.cars.count; i++) {

		//move cars when road shrinks
		if (game.grass.ptr[0].width > game.grass.ptr[1].width) {
			if (game.cars.ptr[i].x < game.grass.ptr[0].width) game.cars.ptr[i].x += CAR_MOVE_PIXELS;
			else if (game.cars.ptr[i].x + CAR_WIDTH / 2 > SCREEN_WIDTH - game.grass.ptr[0].width) game.cars.ptr[i].x -= CAR_MOVE_PIXELS;
			if (game.grass.ptr[0].width == MIN_GRASS_WIDTH) game.cars.ptr[i].x -= CAR_MOVE_PIXELS;
		}

		//cars speed acroding to player speed
		game.cars.ptr[i].y += player_car.speed - game.cars.ptr[i].speed;

		//player bumping to cars
		if (player_car.y - CAR_HEIGTH < game.cars.ptr[i].y && player_car.y > game.cars.ptr[i].y && player_car.x + CAR_WIDTH >= game.cars.ptr[i].x && player_car.x <= game.cars.ptr[i].x + CAR_WIDTH) {
			player_car.in_grass = true;
			game.fire.x = player_car.x - FIRE_WIDTH / 2;
			game.fire.y = player_car.y;
			player_car.time = SDL_GetTicks();
			if (time.world_time >= INF_LIVES_TIME) game.lives -= 1;
		}

		//other cars bumping to each other
		for (int k = 0; k < game.cars.count; k++) {
			if (k == i) continue;
			if (game.cars.ptr[i].y - CAR_HEIGTH < game.cars.ptr[k].y && game.cars.ptr[i].y > game.cars.ptr[k].y && game.cars.ptr[i].x + CAR_WIDTH >= game.cars.ptr[k].x && game.cars.ptr[i].x <= game.cars.ptr[k].x + CAR_WIDTH) {
				game.cars.ptr[i].y += 30;
				game.cars.ptr[i].speed -= SPEED_INCREMENT;
			}
		}

		//Check if bullet hit car (when hit removed from map)
		for (int k = 0; k < game.bullets.count; k++) {
			if (game.bullets.ptr[k].y - CAR_HEIGTH < game.cars.ptr[i].y && game.bullets.ptr[k].y > game.cars.ptr[i].y && game.bullets.ptr[k].x + CAR_WIDTH >= game.cars.ptr[i].x && game.bullets.ptr[k].x <= game.cars.ptr[i].x + CAR_WIDTH) {
				//save time when car wash shoted
				if (game.cars.ptr[i].type == NORMAL) {
					game.score_halt = SDL_GetTicks();
				}
				car_vector_delete(&game.cars, i);
				bullet_vector_delete(&game.bullets, k);
			}
		}

		remove_cars_outside_screen(game);
	}
}


void scroll_grass(game_t& game, car_t& player_car) {
	for (int i = 0; i < game.grass.count; i++) {
		game.grass.ptr[i].y += player_car.speed;
	}

	//update traveled distance
	game.traveled_distance += player_car.speed;

	if (game.grass.ptr[game.grass.count - 1].y - GRASS_HEIGHT >= SCREEN_HEIGHT) {
		grass_pop_back(&game.grass);
	}

	if (game.grass.ptr[game.grass.count - 1].y + GRASS_HEIGHT > SCREEN_HEIGHT && game.grass.count < NUMBER_OF_GRASS_TXT + 1) {
		grass_t grass;
		grass.y = game.grass.ptr[game.grass.count - 1].y + GRASS_HEIGHT - SCREEN_HEIGHT - GRASS_HEIGHT;
		grass.width = pop_back(&game.grass_que);

		grass_add_to_front(&game.grass, grass);
	}
}


//increse score when car traveled height of screen
void icrease_score(game_t& game, car_t& car) {

	if (game.traveled_distance >= SCREEN_HEIGHT) {
		game.traveled_distance -= SCREEN_HEIGHT;
		
		//return if score is halted
		if (SDL_GetTicks() - game.score_halt < 3000 && game.score_halt > 0) return;
		game.score_halt = 0;
		game.score += 50;
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


void load_saves(char saves[10][128]) {
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


void show_saves_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[10][128]) {
	SDL_RenderClear(renderer);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	SDL_FillRect(screen, NULL, czarny);
	char text[128];
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	sprintf(text, "Load game");
	DrawString(screen, TEXT_CENTER, 40, text, charset);
	sprintf(text, " press number to load save");
	DrawString(screen, TEXT_CENTER, 60, text, charset);

	for (int i = 0; i < 10; i++) {
		if (saves[i][0] == '\0') break;
		_itoa(i + 1, text, 10);
		strcat_s(text, ") ");
		strcat_s(text, saves[i]);
		DrawString(screen, TEXT_CENTER, 70 + (20 * (i + 1)), text, charset);
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


void load_save(game_t& game, gameTime_t& game_time, car_t& car, char file_name[]) {

	strcat(file_name, ".bin");

	FILE* file = fopen(file_name, "r");
	if (file == NULL) cout << "ERROR WHILE OPENING FILE";
	else {

		free(game.grass.ptr);
		init_grass_vector(&game.grass);
		int old_que_count = 0;
		fread(&old_que_count, sizeof(int), 1, file);

		grass_t saved_grass;
		for (int i = 0; i < old_que_count; i++) {
			fread(&saved_grass, sizeof(grass_t), 1, file);
			grass_push_back(&game.grass, saved_grass);
		}

		free(game.grass_que.ptr);
		init_vector(&game.grass_que);
		old_que_count = 0;
		fread(&old_que_count, sizeof(game.grass_que.count), 1, file);

		int k = 0;
		for (int i = 0; i < old_que_count; i++) {
			fread(&k, sizeof(int), 1, file);
			push_back(&game.grass_que, k);
		}

		free(game.cars.ptr);
		init_car_vector(&game.cars);
		old_que_count = 0;
		fread(&old_que_count, sizeof(int), 1, file);

		car_t saved_car;
		for (int i = 0; i < old_que_count; i++) {
			fread(&saved_car, sizeof(car_t), 1, file);
			car_push_back(&game.cars, saved_car);
		}

		fread(&game.score, sizeof(game.score), 1, file);
		fread(&game.grass_width_on_car_y, sizeof(game.grass_width_on_car_y), 1, file);
		fread(&car, sizeof(car), 1, file);
		fread(&game_time, sizeof(game_time), 1, file);
		game_time.t1 = SDL_GetTicks();
		fclose(file);
	}
}


void save_game(game_t& game, gameTime_t& game_time, car_t& car) {
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
		fwrite(&game.grass, sizeof(game.grass.count), 1, file);
		for (int i = 0; i < game.grass.count; i++) {
			fwrite(&game.grass.ptr[i], sizeof(game.grass.ptr[0]), 1, file);
		}

		fwrite(&game.grass_que.count, sizeof(game.grass_que.count), 1, file);
		for (int i = 0; i < game.grass_que.count; i++) {
			fwrite(&game.grass_que.ptr[i], sizeof(int), 1, file);
		}

		fwrite(&game.cars.count, sizeof(game.cars.count), 1, file);
		for (int i = 0; i < game.cars.count; i++) {
			fwrite(&game.cars.ptr[i], sizeof(game.cars.ptr[0]), 1, file);
		}

		fwrite(&game.score, sizeof(game.score), 1, file);
		fwrite(&game.grass_width_on_car_y, sizeof(game.grass_width_on_car_y), 1, file);
		fwrite(&car, sizeof(car), 1, file);
		fwrite(&game_time, sizeof(game_time), 1, file);
		fclose(file);
	}
}


int generate_random_x_on_road(game_t* game) {
	int road_width = SCREEN_WIDTH - 2 * game->grass.ptr[0].width;
	int x = (rand() & road_width) + 1;
	int left = x % CAR_MOVE_PIXELS;
	x = x - left;
	if (x > SCREEN_WIDTH / 2) x -= GRASS_WIDTH;
	return x;
}


void pick_up_heart(game_t& game, car_t& player_car) {
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = game.heart.x;
	rightA = game.heart.x + HEART_WIDTH;
	topA = game.heart.y;
	bottomA = game.heart.y + HEART_HEIGTH;

	leftB = player_car.x;
	rightB = player_car.x + CAR_WIDTH;
	topB = CAR_Y;
	bottomB = CAR_Y + CAR_HEIGTH;

	bool picked = true;

	if (bottomA <= topB)
	{
		picked = false;
	}

	if (topA >= bottomB)
	{
		picked = false;
	}

	if (rightA <= leftB)
	{
		picked = false;
	}

	if (leftA >= rightB)
	{
		picked = false;
	}

	if (picked == true) {
		game.lives++;
		game.heart.y = OUT_OF_SCREEN;
	}
}


void generate_random_car(textures_t textures, game_t* game, car_t& player_car) {
	if (game->cars.count <= MAX_CARS) {
		int n = (rand() % 99) + 1;
		if (n == 1) {
			car_t random_car;

			int x = generate_random_x_on_road(game);
			random_car.x = game->grass.ptr[0].width + x;
			random_car.y = 0 - CAR_HEIGTH / 2;
			random_car.speed = player_car.speed - SPEED_INCREMENT;

			int type = rand() % 2;
			if (type == NORMAL) {
				random_car.type = NORMAL;
			}
			else {
				random_car.type = HOSTILE;
			}
			car_push_back(&game->cars, random_car);
		}
	}
}


void generate_random_heart(game_t& game, gameTime_t& time) {
	if (time.world_time <= INF_LIVES_TIME && game.heart.y == -1) {
		int n = (rand() % 1000) + 1;
		if (n == 1) {
			int x = generate_random_x_on_road(&game);
			game.heart.x = game.grass.ptr[0].width + x;
			game.heart.y = 0;
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



//respawn car after 3 seconds after death
void respawn_car(car_t& player_car, game_t& game) {

	if (game.grass_width_on_car_y == MIN_GRASS_WIDTH) {
		player_car.x = CAR_X + CAR_WIDTH;
	}
	else {
		player_car.x = CAR_X;
	}

	if (SDL_GetTicks() - player_car.time >= CAR_RESPAWN_TIME) {
		player_car.y = CAR_Y;
		player_car.speed = START_SPEED;
		player_car.in_grass = false;
		game.fire = { 0 };
	}
	else {
		player_car.y = SCREEN_HEIGHT;
	}
}


void manage_car_in_grass(car_t& player_car, game_t& game, gameTime_t& time) {
	player_car.speed = 0;
	player_car.in_grass = true;
	game.fire.y = player_car.y;
	player_car.time = SDL_GetTicks();
	if (time.world_time >= INF_LIVES_TIME) {
		game.lives -= 1;
	}
}

//check if car is in grass 
void check_for_grass_colision(car_t& player_car, game_t& game, gameTime_t& time) {

	if (player_car.x + CAR_WIDTH / 2 <= game.grass_width_on_car_y) {
		manage_car_in_grass(player_car, game, time);
		game.fire.x = player_car.x - FIRE_WIDTH + CAR_WIDTH / 2;
	}
	else if (player_car.x + CAR_WIDTH / 2 >= SCREEN_WIDTH - game.grass_width_on_car_y) {
		manage_car_in_grass(player_car, game, time);
		game.fire.x = player_car.x;
	}
	else if (game.grass_width_on_car_y == MIN_GRASS_WIDTH) {
		if (player_car.x == CAR_X) {
			manage_car_in_grass(player_car, game, time);
			game.fire.x = player_car.x;
		};
	}

	if (player_car.in_grass) {
		respawn_car(player_car, game);
	}
}



void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	DrawRectangle(screen, SCREEN_WIDTH - IMPLEMENTED_WIDTH, SCREEN_HEIGHT - IMPLEMENTED_HEIGHT, IMPLEMENTED_WIDTH, IMPLEMENTED_HEIGHT, czarny, niebieski);
	sprintf(text, "a b c d e f g");
	DrawString(screen, screen->w - IMPLEMENTED_WIDTH, screen->h - IMPLEMENTED_HEIGHT, text, charset);
	sprintf(text, "e g");
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
	for (int i = 0; i < game.grass.count; ++i)
	{
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

		//saving grass width on car y position (used for car grass collision)
		if (roadRect_right.y <= CAR_Y + CAR_HEIGTH && roadRect_right.y >= CAR_Y && game.grass.ptr[i].width > 0) {
			game.grass_width_on_car_y = game.grass.ptr[i].width;
		}

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


void generate_start_road(game_t& game) {
	if (game.grass.count == 0) {
		for (int i = 0; i < NUMBER_OF_GRASS_TXT; i++) {

			grass_t grass;
			grass.y = GRASS_HEIGHT * i;
			grass.width = 6 * GRASS_WIDTH;

			grass_push_back(&game.grass, grass);
		}
	}
}


// n = 1 - zwê¿enie drogi
// n = 2 - rozszerzenie drogi
// n = 3 - bez zmian
void generate_road_que(game_t& game) {

	if (game.grass_que.count == 0) {
		int n = (rand() % 3) + 1;
		if (n == 1) {
			if (game.grass.ptr[0].width == MAX_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MAX_GRASS_WIDTH);
				}
			}
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
			if (game.grass.ptr[0].width == MIN_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MIN_GRASS_WIDTH);
				}
			}
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
		else {
			for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
			{
				push_back(&game.grass_que, game.grass.ptr[0].width);
			}
		}
	}
}


void cap_fps(fps_t& game_fps, car_t& car) {
	int delta = (SDL_GetTicks() - game_fps.start_loop);
	if (delta < game_fps.desired_delta) {
		SDL_Delay(game_fps.desired_delta - delta);
	}
}


void render_legend(SDL_Surface* screen, SDL_Surface* charset, gameTime_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game, colors_t& colors) {

	char text[128];

	DrawRectangle(screen, 0, 0, SCREEN_WIDTH, LEGEND_HEIGHT, colors.czarny, colors.niebieski);
	sprintf(text, "Igor Stadnicki 193435");
	DrawString(screen, TEXT_CENTER, 5, text, charset);

	if (time.world_time >= 60) {
		sprintf(text, "Time = %.1lf s  score: %.0lf  lives: %d", time.world_time, game.score, game.lives);
	}
	else {
		sprintf(text, "Time = %.1lf s  score: %.0lf  lives: unlimited for 60s (%d)", time.world_time, game.score, game.lives);
	}
	DrawString(screen, TEXT_CENTER, 20, text, charset);

	sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie, n - nowa gra");
	DrawString(screen, TEXT_CENTER, 35, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

	SDL_Rect roadRect_left;
	roadRect_left.x = 0;
	roadRect_left.y = 0;
	roadRect_left.w = SCREEN_WIDTH;
	roadRect_left.h = LEGEND_HEIGHT;

	SDL_RenderCopy(renderer, scrtex, &roadRect_left, &roadRect_left);
}


//calculates game time
void calculate_time(gameTime_t& time) {
	time.t2 = SDL_GetTicks();
	time.world_time += (time.t2 - time.t1) * 0.001;
	time.t1 = time.t2;
}


//fire a bullet (can fire every 0.5 second)
void fire_bullet(game_t& game, car_t& player_car) {

	if (SDL_GetTicks() - game.bullets.ptr[game.bullets.count - 1].time < 500 && game.bullets.count > 0) return;
	bullet_t bullet;
	bullet.time = SDL_GetTicks();
	bullet.x = player_car.x + CAR_WIDTH / 2;
	bullet_push_back(&game.bullets, bullet);
}


// Obs³uga zdarzeñ
void events_handling(SDL_Event& event, car_t& car, game_t& game, gameTime_t& time) {
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LEFT && !game.pause && car.in_grass == false) {
				if (car.x + CAR_MOVE_PIXELS > 0)
					car.x -= CAR_MOVE_PIXELS;
			}
			else if (event.key.keysym.sym == SDLK_RIGHT && !game.pause && car.in_grass == false) {
				if (car.x + CAR_MOVE_PIXELS + CAR_WIDTH / 2 < SCREEN_WIDTH)
					car.x += CAR_MOVE_PIXELS;
			}
			else if (event.key.keysym.sym == SDLK_ESCAPE) {
				game.running = false;
			}
			else if (event.key.keysym.sym == SDLK_UP && !game.pause && !car.in_grass && car.in_grass == false) {

				if (car.speed + SPEED_INCREMENT <= MAX_SPEED)
					car.speed += SPEED_INCREMENT;
			}
			else if (event.key.keysym.sym == SDLK_DOWN && !game.pause && car.in_grass == false) {
				if (car.speed - SPEED_INCREMENT >= 0)
					car.speed -= SPEED_INCREMENT;
			}
			else if (event.key.keysym.sym == SDLK_n) restart_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_p) stop_game(car, time, game);
			else if (event.key.keysym.sym == SDLK_s) save_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_l) game.save_screen = true;
			else if (event.key.keysym.sym == SDLK_SPACE) fire_bullet(game, car);
			break;
		case SDL_QUIT:
			game.running = false;
			break;
		}
	}
}


void stop_game(car_t& car, gameTime_t& time, game_t& game) {
	game.pause == false ? game.pause = true : game.pause = false;
	time.t1 = SDL_GetTicks();
}


void restart_game(game_t& game, gameTime_t& time, car_t& car) {
	free(game.grass_que.ptr);
	init_vector(&game.grass_que);
	free(game.cars.ptr);
	init_car_vector(&game.cars);
	free(game.grass.ptr);
	init_grass_vector(&game.grass);
	generate_start_road(game);
	game.score = 0;
	game.lives = START_LIVES;
	time.world_time = 0;
	time.t1 = SDL_GetTicks();
	time.t2 = 0;
	car.speed = START_SPEED;
	car.x = CAR_X;
	car.y = CAR_Y;
	cout << game.lives << endl;
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


void render_bullet(SDL_Renderer* renderer, game_t& game, SDL_Texture* bullet_texture) {

	for (int i = 0; i < game.bullets.count; i++) {
		SDL_Rect bullet_rect = { game.bullets.ptr[i].x, game.bullets.ptr[i].y, BULLET_WIDTH, BULLET_HEIGTH };
		SDL_RenderCopy(renderer, bullet_texture, NULL, &bullet_rect);
	}
	move_bullets(game);
}


void render_fire(SDL_Renderer* renderer, game_t& game, SDL_Texture* fire) {

	if (game.fire.x != 0 && game.fire.y != 0) {
		SDL_Rect fire_rect = { game.fire.x, game.fire.y, FIRE_WIDTH, FIRE_HEIGTH };
		SDL_RenderCopy(renderer, fire, NULL, &fire_rect);
	}
}


void render_heart(SDL_Renderer* renderer, game_t& game, SDL_Texture* heart, car_t& player_car) {

	SDL_Rect heart_rect = { game.heart.x, game.heart.y, HEART_WIDTH, HEART_HEIGTH };
	SDL_RenderCopy(renderer, heart, NULL, &heart_rect);

	if (game.heart.y >= 0) {
		game.heart.y += player_car.speed;
	}
	if (game.heart.y >= SCREEN_HEIGHT) {
		game.heart.y = OUT_OF_SCREEN;
	}
}


void init_colors(colors_t& colors, SDL_Surface* screen) {
	colors.czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	colors.niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
}


void show_gameover_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, game_t& game) {
	SDL_FillRect(screen, NULL, colors.czarny);
	char text[128];
	sprintf(text, "GAME OVER SCORE: %.lf", game.score);
	DrawString(screen, TEXT_CENTER, 50, text, charset);
	sprintf(text, "Click n for new game");
	DrawString(screen, TEXT_CENTER, 66, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


void show_pause_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
	SDL_FillRect(screen, NULL, colors.czarny);
	char text[128];
	sprintf(text, "GAME PAUSED");
	DrawString(screen, TEXT_CENTER, 50, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}


void load_picked_save(SDL_Event& event, game_t& game, gameTime_t& time, car_t& player_car, char  saves[10][128]) {
	int save_number = event.key.keysym.sym - '0' - 1;
	if (save_number >= 0 && save_number < SAVES_NUMBER) {
		load_save(game, time, player_car, saves[save_number]);
		game.save_screen = false;
	}
}