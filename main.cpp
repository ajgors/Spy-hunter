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
#include <vector>
#include "vector.h"
#include "cars_vector.h"
#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

struct car_t {
	int x = CAR_X;
	int y = CAR_Y;
	int speed = 2;
	bool in_grass = false;
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

struct game_t {
	vector_t grass_que;
	vector_t grass;
	int grass_width_on_car_y = 0;
	bool running = true;
	bool pause = false;
	bool save_screen = false;
	double score = 0;
	car_vector_t cars;

	vector<grass_t> grass_vector;
};

struct textures_t {
	SDL_Texture* red_car = NULL;
	SDL_Texture* grass = NULL;
	SDL_Texture* blue_car = NULL;
};


void clear_cars_outside_screen(game_t& game) {
	car_vector_t tmp_cars;
	init_car_vector(&tmp_cars);

	for (int i = 0; i < game.cars.count; i++) {
		if (game.cars.ptr[i].y > SCREEN_HEIGHT) continue;
		if (game.cars.ptr[i].y < -CAR_HEIGTH) continue;
		if (game.cars.ptr[i].y - CAR_HEIGTH < SCREEN_HEIGHT) {
			car_push_back(&tmp_cars, game.cars.ptr[i]);
		}
		else if (game.cars.ptr[i].y + CAR_HEIGTH > LEGEND_HEIGHT) {
			car_push_back(&tmp_cars, game.cars.ptr[i]);
		}
	}
	free(game.cars.ptr);
	init_car_vector(&game.cars);
	game.cars = tmp_cars;
}


void update_cars_speed(game_t& game) {
	for (int i = 0; i < game.cars.count; i++) {
		int n = (rand() % 100) + 1;

		if (n == 1 && game.cars.ptr[i].speed + 1 < 4) game.cars.ptr[i].speed += 1;
		if (n == 100 && game.cars.ptr[i].speed - 1 > 0) game.cars.ptr[i].speed -= 1;
	}
}


void render_cars(SDL_Renderer* renderer, textures_t& textures, game_t& game) {
	for (int i = 0; i < game.cars.count; i++) {
		render_car(game.cars.ptr[i], renderer, textures.blue_car);
	}
}


void manage_cars_position(game_t& game, car_t& player_car) {

	for (int i = 0; i < game.cars.count; i++) {
		if (game.grass.ptr[0] > game.grass.ptr[1]) {
			if (game.cars.ptr[i].x < game.grass.ptr[0]) game.cars.ptr[i].x += CAR_MOVE_PIXELS;
			else if (game.cars.ptr[i].x + CAR_WIDTH / 2 > SCREEN_WIDTH - game.grass.ptr[0]) game.cars.ptr[i].x -= CAR_MOVE_PIXELS;
			if (game.grass.ptr[0] == MIN_GRASS_WIDTH) game.cars.ptr[i].x -= CAR_MOVE_PIXELS;
		}
		
		if (player_car.speed == game.cars.ptr[i].speed + 2) game.cars.ptr[i].y += 4;
		if (player_car.speed == game.cars.ptr[i].speed + 1) game.cars.ptr[i].y += 2;
		if (player_car.speed == game.cars.ptr[i].speed - 1) game.cars.ptr[i].y -= 4;
		if (player_car.speed == game.cars.ptr[i].speed - 2) game.cars.ptr[i].y -= 6;
		if (player_car.speed == game.cars.ptr[i].speed - 3) game.cars.ptr[i].y -= 10;

		if (player_car.y - CAR_HEIGTH < game.cars.ptr[i].y && player_car.y > game.cars.ptr[i].y && player_car.x + CAR_WIDTH >= game.cars.ptr[i].x && player_car.x <= game.cars.ptr[i].x + CAR_WIDTH) player_car.speed = 0;

		for (int k = 0; k < game.cars.count; k++) {
			if (k == i) continue;
			if (game.cars.ptr[i].y - CAR_HEIGTH < game.cars.ptr[k].y && game.cars.ptr[i].y > game.cars.ptr[k].y && game.cars.ptr[i].x + CAR_WIDTH >= game.cars.ptr[k].x && game.cars.ptr[i].x <= game.cars.ptr[k].x + CAR_WIDTH) {

				game.cars.ptr[i].y += 30;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	srand(time(nullptr));
	gameTime_t time;
	car_t player_car;
	fps_t game_fps;

	game_t game;
	init_car_vector(&game.cars);
	init_vector(&game.grass_que);
	init_vector(&game.grass);

	SDL_Event event;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* screen, * charset = NULL;
	SDL_Texture* scrtex;
	textures_t textures;

	if (init(window, renderer, screen, scrtex)) {
		load_charset(charset);
		textures.red_car = load_texture("car_red.bmp", renderer);
		textures.grass = load_texture("grass.bmp", renderer);
		textures.blue_car = load_texture("car_blue.bmp", renderer);
		if (charset == NULL || textures.grass == NULL || textures.red_car == NULL) {
			free_textures(textures);;
			free_memory(screen, scrtex, renderer, window);
			SDL_Quit();
		}
	};

	generate_start_road(game);

	while (game.running)
	{
		if (!game.pause && !game.save_screen) {
			game_fps.start_loop = SDL_GetTicks();
			SDL_RenderClear(renderer);
			generate_road_que(game);
			calculate_time(time);
			generate_random_car(textures, &game);
			render_grass(game, renderer, textures.grass, player_car);
			check_for_colision(player_car, game);
			render_implemented(screen, charset, scrtex, renderer);
			render_car(player_car, renderer, textures.red_car);
			clear_cars_outside_screen(game);
			update_cars_speed(game);
			render_cars(renderer, textures, game);
			manage_cars_position(game, player_car);
			icrease_score(game, player_car);
			render_legend(screen, charset, time, game_fps, renderer, scrtex, game);



			for (int i = 0; i < game.grass_vector.size(); i++) {
				game.grass_vector[i].y += 4 * player_car.speed;

				/* if (game.grass_vector[i].y >= CAR_Y) {
					cout << game.grass_vector[i].y << endl;
				}*/
			}

			if (game.grass_vector[game.grass_vector.size() - 1].y - GRASS_HEIGHT >= SCREEN_HEIGHT) {
				game.grass_vector.pop_back();
			}

			if (game.grass_vector[game.grass_vector.size() - 1].y + GRASS_HEIGHT > SCREEN_HEIGHT && game.grass_vector.size() < NUMBER_OF_GRASS_TXT + 1) {
				grass_t grass;
				grass.y = game.grass_vector[game.grass_vector.size() - 1].y + GRASS_HEIGHT - SCREEN_HEIGHT - GRASS_HEIGHT;
				game.grass_vector.insert(game.grass_vector.begin(), grass);
				add_from_que_to_road(game, player_car);
			}

			SDL_RenderPresent(renderer);
		}
		else if (game.save_screen) {
			char saves[10][128] = { 0 };
			load_saves(saves);
			load_save_screen(screen, charset, scrtex, renderer, event, saves);
			int save_number = event.key.keysym.sym - '0' - 1;

			if (save_number >= 0 && save_number < 10) {
				load_save(game, time, player_car, saves[save_number]);
				game.save_screen = false;
				int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
				SDL_FillRect(screen, NULL, czarny);
				SDL_RenderClear(renderer);
			}
		}
		else if (!game.save_screen && game.pause) {
			int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			SDL_FillRect(screen, NULL, czarny);
			char text[128];
			sprintf(text, "GAME PAUSED");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 50, text, charset);
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
			SDL_FillRect(screen, NULL, czarny);

		}
		events_handling(event, player_car, game, time);
		cap_fps(game_fps, player_car);
	}
	free_textures(textures);
	free_memory(screen, scrtex, renderer, window);
	SDL_Quit();

	return 1;
}


void icrease_score(game_t& game, car_t& car) {
	game.score += 1 * car.speed;
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
	};
};


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screensprite
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


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
};


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


void load_save_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[10][128]) {
	SDL_RenderClear(renderer);
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	SDL_FillRect(screen, NULL, czarny);
	char text[128];
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	sprintf(text, "Load game");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 40, text, charset);
	sprintf(text, " press number to load save");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 60, text, charset);

	for (int i = 0; i < 10; i++) {
		if (saves[i][0] == '\0') break;
		_itoa(i + 1, text, 10);
		strcat_s(text, ") ");
		strcat_s(text, saves[i]);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 70 + (20 * (i + 1)), text, charset);
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
};


void load_save(game_t& game, gameTime_t& game_time, car_t& car, char file_name[]) {

	strcat(file_name, ".bin");

	FILE* file = fopen(file_name, "r");
	if (file == NULL) {
		cout << "b³¹d w otwarciu pliku";
	}
	else {
		for (int i = 0; i < game.grass.count; i++) {
			fread(&game.grass.ptr[i], sizeof(int), 1, file);
		}
		fread(&game.grass.count, sizeof(game.grass.count), 1, file);
		fread(&game.grass.allocated_size, sizeof(game.grass.allocated_size), 1, file);

		free(game.grass_que.ptr);
		init_vector(&game.grass_que);
		int old_que_count = 0;
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

		cout << "uda³o siê wczytaæ" << endl;
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
	if (file == NULL) {
		cout << "FAILED OPENING FILE" << endl;
	}
	else {
		for (int i = 0; i < game.grass.count; i++) {
			fwrite(&game.grass.ptr[i], sizeof(int), 1, file);
		}
		fwrite(&game.grass.count, sizeof(game.grass.count), 1, file);
		fwrite(&game.grass.allocated_size, sizeof(game.grass.allocated_size), 1, file);


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

		cout << "Saved at: " << buffer << endl;
		fclose(file);
	}


};


void generate_random_car(textures_t textures, game_t* game) {
	if (game->cars.count < 7) {
		int n = (rand() % 99) + 1;
		if (n == 1) {
			car_t random_car;

			int road_width = SCREEN_WIDTH - 2 * game->grass.ptr[0];
			int x = (rand() & road_width) + 1;
			int left = x % CAR_MOVE_PIXELS;
			x = x - left;
			if (x > SCREEN_WIDTH / 2) x -= GRASS_WIDTH;

			random_car.x = game->grass.ptr[0] + x;
			random_car.y = 0 - CAR_HEIGTH / 2;

			car_push_back(&game->cars, random_car);
		}
	}
};



void free_textures(textures_t& textures)
{
	SDL_DestroyTexture(textures.red_car);
	SDL_DestroyTexture(textures.grass);
}


void load_charset(SDL_Surface*& charset)
{
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) cout << SDL_GetError();
	SDL_SetColorKey(charset, true, 0x000000);
}


void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window)
{
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}


int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex)
{
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


void check_for_colision(car_t& car, game_t& game)
{
	if (car.x + CAR_WIDTH / 2 <= game.grass_width_on_car_y) {
		car.speed = 0;
		car.in_grass = true;
	}
	else if (car.x + CAR_WIDTH / 2 >= SCREEN_WIDTH - game.grass_width_on_car_y) {
		car.speed = 0;
		car.in_grass = true;
	}
	else if (game.grass_width_on_car_y == MIN_GRASS_WIDTH) {
		if (car.x == CAR_X) {
			car.speed = 0;
			car.in_grass = true;
		};
	}
}


void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
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


void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car)
{
	if (game.grass_vector.size() == 0) {
		for (int i = 0; i < NUMBER_OF_GRASS_TXT; i++) {

			grass_t grass;
			grass.y = GRASS_HEIGHT * i;

			game.grass_vector.push_back(grass);
		}
	}

	for (int i = 0; i < game.grass_vector.size(); ++i)
	{
		SDL_Rect roadRect_left;
		roadRect_left.x = 0;
		roadRect_left.y = game.grass_vector[i].y;
		roadRect_left.w = game.grass.ptr[i];
		roadRect_left.h = GRASS_HEIGHT;

		SDL_Rect roadRect_right;
		roadRect_right.x = SCREEN_WIDTH - game.grass.ptr[i];
		roadRect_right.y = game.grass_vector[i].y;
		roadRect_right.w = game.grass.ptr[i];
		roadRect_right.h = GRASS_HEIGHT;

		//saving grass width on car y position (used for car grass collision)
		if (roadRect_right.y <= car.y + CAR_HEIGTH && roadRect_right.y >= car.y && game.grass.ptr[i] > 0) {
			game.grass_width_on_car_y = game.grass.ptr[i];
		}

		if (game.grass.ptr[i] == MIN_GRASS_WIDTH) {
			SDL_Rect roadRect_middle;
			roadRect_middle.x = SCREEN_WIDTH / 2 - GRASS_WIDTH / 2;
			roadRect_middle.y = game.grass_vector[i].y;
			roadRect_middle.w = MIN_GRASS_WIDTH;
			roadRect_middle.h = GRASS_HEIGHT;
			SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_middle);
		}

		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_left);
		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_right);
	}
}


void generate_start_road(game_t& game)
{
	for (int i = 0; i < SCREEN_HEIGHT / GRASS_HEIGHT; ++i)
		push_back(&game.grass, 6 * GRASS_WIDTH);
}


void add_from_que_to_road(game_t& game, car_t& car)
{
	if (car.speed > 0) {
		vector_t grass_tmp;
		init_vector(&grass_tmp);
		int r = pop_back(&game.grass_que);
		push_back(&grass_tmp, r);
		for (int i = 0; i < game.grass.count - 1; ++i)
		{
			push_back(&grass_tmp, game.grass.ptr[i]);
		}
		free(game.grass.ptr);
		game.grass.ptr = grass_tmp.ptr;
	}
}


void generate_road_que(game_t& game)
{
	// n = 1 - zwê¿enie drogi
	// n = 2 - rozszerzenie drogi
	// n = 3 - bez zmian

	if (game.grass_que.count == 0) {
		int n = (rand() % 3) + 1;
		if (n == 1) {
			if (game.grass.ptr[0] == MAX_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MAX_GRASS_WIDTH);
				}
			}
			else {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
				{
					if (i < NUMBER_OF_GRASS_TXT && game.grass.ptr[0] + 3 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0] + 3 * GRASS_WIDTH);
					}
					else if (i >= NUMBER_OF_GRASS_TXT && i < 2 * NUMBER_OF_GRASS_TXT && game.grass.ptr[0] + 2 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {

						push_back(&game.grass_que, game.grass.ptr[0] + 2 * GRASS_WIDTH);
					}
					else if (i >= 2 * NUMBER_OF_GRASS_TXT && i < NUMBER_OF_GRASS_TXT * 3 && game.grass.ptr[0] + 1 * GRASS_WIDTH <= MAX_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0] + 1 * GRASS_WIDTH);
					}
					else {
						push_back(&game.grass_que, MAX_GRASS_WIDTH);
					}
				}
			}
		}
		else if (n == 2) {
			if (game.grass.ptr[0] == MIN_GRASS_WIDTH) {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT; ++i)
				{
					push_back(&game.grass_que, MIN_GRASS_WIDTH);
				}
			}
			else {
				for (int i = 0; i < NUMBER_OF_GRASS_TXT * 3; ++i)
				{
					if (i < NUMBER_OF_GRASS_TXT && game.grass.ptr[0] - 3 * GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0] - 3 * GRASS_WIDTH);
					}
					else if (i >= NUMBER_OF_GRASS_TXT && i < 2 * NUMBER_OF_GRASS_TXT && game.grass.ptr[0] - 2 * GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0] - 2 * GRASS_WIDTH);
					}
					else if (i >= 2 * NUMBER_OF_GRASS_TXT && i < NUMBER_OF_GRASS_TXT * 3 && game.grass.ptr[0] - GRASS_WIDTH >= MIN_GRASS_WIDTH) {
						push_back(&game.grass_que, game.grass.ptr[0] - 1 * GRASS_WIDTH);
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
				push_back(&game.grass_que, game.grass.ptr[0]);
			}
		}
	}
}


void cap_fps(fps_t& game_fps, car_t& car)
{
	int delta = (SDL_GetTicks() - game_fps.start_loop) /** (car.speed - 1)*/;
	if (delta < game_fps.desired_delta) {
		SDL_Delay(game_fps.desired_delta - delta);
	}
}


void render_legend(SDL_Surface* screen, SDL_Surface* charset, gameTime_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game)
{

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	DrawRectangle(screen, 0, 0, SCREEN_WIDTH, LEGEND_HEIGHT, czarny, niebieski);
	sprintf(text, "Igor Stadnicki 193435");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 5, text, charset);
	;
	sprintf(text, "Time = %.1lf s  score: %.0lf", time.world_time, game.score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);

	sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie, n - nowa gra");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 35, text, charset);


	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

	SDL_Rect roadRect_left;
	roadRect_left.x = 0;
	roadRect_left.y = 0;
	roadRect_left.w = SCREEN_WIDTH;
	roadRect_left.h = LEGEND_HEIGHT;

	SDL_RenderCopy(renderer, scrtex, &roadRect_left, &roadRect_left);
}


void calculate_time(gameTime_t& time)
{
	time.t2 = SDL_GetTicks();
	time.world_time += (time.t2 - time.t1) * 0.001;
	time.t1 = time.t2;
}


// Obs³uga zdarzeñ
void events_handling(SDL_Event& event, car_t& car, game_t& game, gameTime_t& time)
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LEFT && !game.pause) {
				if (car.x + CAR_MOVE_PIXELS > 0)
				{
					car.in_grass = false;
					car.x -= CAR_MOVE_PIXELS;
				}
			}
			else if (event.key.keysym.sym == SDLK_RIGHT && !game.pause) {
				if (car.x + CAR_MOVE_PIXELS + CAR_WIDTH / 2 < SCREEN_WIDTH)
				{
					car.in_grass = false;
					car.x += CAR_MOVE_PIXELS;
				}
			}
			else if (event.key.keysym.sym == SDLK_ESCAPE) {
				game.running = false;
			}
			else if (event.key.keysym.sym == SDLK_UP && !game.pause && !car.in_grass) {

				if (car.speed + 1 <= 4)
					car.speed += 1;
			}
			else if (event.key.keysym.sym == SDLK_DOWN && !game.pause) {
				if (car.speed - 1 >= 0)
					car.speed -= 1;
			}
			else if (event.key.keysym.sym == SDLK_n) restart_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_p) stop_game(car, time, game);
			else if (event.key.keysym.sym == SDLK_s) save_game(game, time, car);
			else if (event.key.keysym.sym == SDLK_l) game.save_screen = true;
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
	game.grass_que.count = 0;
	free(game.grass.ptr);
	init_vector(&game.grass);
	game.grass.count = 0;
	generate_start_road(game);
	game.score = 0;
	time.world_time = 0;
	time.t1 = SDL_GetTicks();
	time.t2 = 0;
	car.speed = 2;
	car.x = CAR_X;
	car.y = CAR_Y;
}


// Renderowanie samochodu
void render_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture)
{
	SDL_Rect carRect;
	carRect.x = car.x;
	carRect.y = car.y;
	carRect.w = CAR_WIDTH;
	carRect.h = CAR_HEIGTH;
	SDL_RenderCopy(renderer, carTexture, nullptr, &carRect);
}
