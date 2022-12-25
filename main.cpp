#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include "main.h"

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#include <iostream>
#include <cstdlib>
#include <vector>
#include "vector.h"
#include <stack>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
#define FPS 30
#define LEGEND_HEIGHT 50
#define GRASS_HEIGHT 32
#define GRASS_WIDTH 20
#define CAR_HEIGTH 90
#define CAR_WIDTH 90
#define CAR_X (SCREEN_WIDTH / 2) - (CAR_WIDTH / 2)
#define CAR_Y (SCREEN_HEIGHT / 2) + (LEGEND_HEIGHT / 2)
#define ERROR 0
#define SUCCESS 1


struct car_t {
	int x = CAR_X;
	int y = CAR_Y;
	double speed = 1;
};


struct gameTime_t {
	double worldTime = SDL_GetTicks();
	int t1 = 0;
	int t2 = 0;
};


struct fps_t {
	int fps = FPS;
	int desired_delta = 1000 / fps;
	int start_loop = 0;
};


struct game_t {
	vector_t que;
	vector_t road;
	bool running = true;
	int road_width = 0;
	double score = 0;
	bool stop = false;
};



void renderCar(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture);
void events_handling(SDL_Event& event, bool& running, car_t& car, game_t& game, gameTime_t& time);
void calculate_time(gameTime_t& time);
void render_legend(SDL_Surface* screen, SDL_Surface* charset, gameTime_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game);
void cap_fps(fps_t& game_fps, car_t& car);
void generate_road_que(game_t& game);
void add_from_que_to_road(game_t& game);
void generate_start_road(game_t& game);
void restart_game(game_t& game, gameTime_t& time);
void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car);
void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer);
void stop_game(car_t& car, gameTime_t& time, game_t& game);

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
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


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
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


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
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



void check_for_colision(car_t& car, game_t& game);

int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex);

void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window);

int load_charset(SDL_Surface*& charset);



struct textures_t {
	SDL_Texture* carTexture = NULL;
	SDL_Texture* grass_texture = NULL;
};


SDL_Texture* load_texture(char s[], SDL_Renderer* renderer) {
	SDL_Surface* carSurface = SDL_LoadBMP(s);
	if (carSurface == nullptr)
	{
		return NULL;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, carSurface);
	SDL_FreeSurface(carSurface);
	if (texture == nullptr)
	{
		return NULL;
	}
	return texture;
}


int main(int argc, char* argv[])
{
	std::srand(std::time(nullptr));
	gameTime_t time;
	car_t car;
	fps_t game_fps;

	game_t game;
	init_vector(&game.que);
	init_vector(&game.road);

	SDL_Event event;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* screen, * charset;
	SDL_Texture* scrtex;
	textures_t textures;


	if (init(window, renderer, screen, scrtex)) {
		load_charset(charset);
		textures.carTexture = load_texture("carasd.bmp", renderer);
		textures.grass_texture = load_texture("grass.bmp", renderer);
		if (charset == NULL || textures.grass_texture == NULL || textures.carTexture == NULL) {
			SDL_DestroyTexture(textures.carTexture);
			SDL_DestroyTexture(textures.grass_texture);
			free_memory(screen, scrtex, renderer, window);
			SDL_Quit();
		}
	};

	generate_start_road(game);

	// PÍtla g≥Ûwna gry
	while (game.running)
	{
		if (!game.stop) {
			game_fps.start_loop = SDL_GetTicks();
			calculate_time(time);
			render_legend(screen, charset, time, game_fps, renderer, scrtex, game);
			render_grass(game, renderer, textures.grass_texture, car);
			render_implemented(screen, charset, scrtex, renderer);
			renderCar(car, renderer, textures.carTexture);
			game.score += 1 * car.speed;
			SDL_RenderPresent(renderer);
			cap_fps(game_fps, car);
			generate_road_que(game);

			if (car.speed > 0) {
				add_from_que_to_road(game);
			}
			std::cout << car.speed << std::endl;
			check_for_colision(car, game);
		}
		events_handling(event, game.running, car, game, time);

	}

	// Czyszczenie zasobÛw
	SDL_DestroyTexture(textures.carTexture);
	SDL_DestroyTexture(textures.grass_texture);
	free_memory(screen, scrtex, renderer, window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

int load_charset(SDL_Surface*& charset)
{
	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		return NULL;
	};
	SDL_SetColorKey(charset, true, 0x000000);
	return SUCCESS;
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
	if (car.x + CAR_WIDTH / 2 <= game.road_width) {
		car.speed = 0;
	}
	else if (car.x + CAR_WIDTH / 2 >= SCREEN_WIDTH - game.road_width) {
		car.speed = 0;
	}
}

void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	DrawRectangle(screen, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 20, 80, 20, czarny, niebieski);
	sprintf(text, "a b c d e f g");
	DrawString(screen, screen->w - 80, screen->h - 20, text, charset);
	sprintf(text, "e g");
	DrawString(screen, screen->w - 80, screen->h - 10, text, charset);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

	SDL_Rect roadRect_left;
	roadRect_left.x = SCREEN_WIDTH - 80;
	roadRect_left.y = SCREEN_HEIGHT - 20;
	roadRect_left.w = 80;
	roadRect_left.h = 20;


	SDL_RenderCopy(renderer, scrtex, &roadRect_left, &roadRect_left);
}


void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car)
{

	for (int i = 0; i < game.road.count; ++i)
	{
		SDL_Rect roadRect_left;
		roadRect_left.x = 0;
		roadRect_left.y = GRASS_HEIGHT * i + LEGEND_HEIGHT;
		roadRect_left.w = game.road.ptr[i] * GRASS_WIDTH;
		roadRect_left.h = GRASS_HEIGHT;

		SDL_Rect roadRect_right;
		roadRect_right.x = SCREEN_WIDTH - game.road.ptr[i] * GRASS_WIDTH;
		roadRect_right.y = GRASS_HEIGHT * i + LEGEND_HEIGHT;
		roadRect_right.w = game.road.ptr[i] * GRASS_WIDTH;
		roadRect_right.h = GRASS_HEIGHT;

		if (roadRect_right.y + 45 >= car.y) {
			game.road_width = roadRect_right.w;
		}

		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_left);
		SDL_RenderCopy(renderer, roadTexture, nullptr, &roadRect_right);
	}

}


void generate_start_road(game_t& game)
{
	for (int i = 0; i < SCREEN_HEIGHT / GRASS_HEIGHT; ++i)
	{
		push_back(&game.road, 6);
	}
}


void add_from_que_to_road(game_t& game)
{
	vector_t roadTmp;
	init_vector(&roadTmp);
	int r = pop_back(&game.que);
	push_back(&roadTmp, r);
	for (int i = 0; i < game.road.count - 1; ++i)
	{
		push_back(&roadTmp, game.road.ptr[i]);
	}
	free(game.road.ptr);
	game.road.ptr = roadTmp.ptr;
}


void generate_road_que(game_t& game)
{
	if (game.que.count == 0) {
		int n = (rand() % 3) + 1;
		if (n == 1) {
			if (game.road.ptr[0] == 8) {
				for (int i = 0; i < SCREEN_HEIGHT / GRASS_HEIGHT; ++i)
				{
					push_back(&game.que, game.road.ptr[0]);
				}
			}
			else {
				for (int i = 0; i < (SCREEN_HEIGHT / GRASS_HEIGHT) * 3; ++i)
				{
					if (i < (SCREEN_HEIGHT / GRASS_HEIGHT) && game.road.ptr[0] + 3 <= 8) {
						push_back(&game.que, game.road.ptr[0] + 3);
					}
					else if (i >= (SCREEN_HEIGHT / GRASS_HEIGHT) && i < 2 * (SCREEN_HEIGHT / GRASS_HEIGHT) && game.road.ptr[0] + 2 <= 8) {

						push_back(&game.que, game.road.ptr[0] + 2);
					}
					else if (i >= 2 * (SCREEN_HEIGHT / GRASS_HEIGHT) && i < (SCREEN_HEIGHT / GRASS_HEIGHT) * 3 && game.road.ptr[0] + 1 <= 8) {
						push_back(&game.que, game.road.ptr[0] + 1);
					}
					else {
						push_back(&game.que, 8);
					}
				}
			}
		}
		else if (n == 2) {
			if (game.road.ptr[0] == 1) {
				for (int i = 0; i < SCREEN_HEIGHT / GRASS_HEIGHT; ++i)
				{
					push_back(&game.que, game.road.ptr[0]);
				}
			}
			else {
				for (int i = 0; i < (SCREEN_HEIGHT / GRASS_HEIGHT) * 3; ++i)
				{
					if (i < (SCREEN_HEIGHT / GRASS_HEIGHT) && game.road.ptr[0] - 3 >= 1) {
						push_back(&game.que, game.road.ptr[0] - 3);
					}
					else if (i >= (SCREEN_HEIGHT / GRASS_HEIGHT) && i < 2 * (SCREEN_HEIGHT / GRASS_HEIGHT) && game.road.ptr[0] - 2 >= 1) {
						push_back(&game.que, game.road.ptr[0] - 2);
					}
					else if (i >= 2 * (SCREEN_HEIGHT / GRASS_HEIGHT) && i < (SCREEN_HEIGHT / GRASS_HEIGHT) * 3 && game.road.ptr[0] - 1 >= 1) {
						push_back(&game.que, game.road.ptr[0] - 1);
					}
					else {
						push_back(&game.que, 1);
					}
				}
			}
		}
		else {
			for (int i = 0; i < (SCREEN_HEIGHT / GRASS_HEIGHT) * 3; ++i)
			{
				push_back(&game.que, game.road.ptr[0]);
			}
		}
	}
}


void cap_fps(fps_t& game_fps, car_t& car)
{
	int delta = (SDL_GetTicks() - game_fps.start_loop) * car.speed;
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
	sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s, score: %lf", time.worldTime, (float)game_fps.fps, game.score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);

	sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 35, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);

}


void calculate_time(gameTime_t& time)
{
	time.t2 = SDL_GetTicks();
	time.worldTime += (time.t2 - time.t1) * 0.001;
	time.t1 = time.t2;
}


// Obs≥uga zdarzeÒ
void events_handling(SDL_Event& event, bool& running, car_t& car, game_t& game, gameTime_t& time)
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LEFT && !game.stop) {
				if (car.x > 0)
				{
					car.x -= 20;
				}
			}
			else if (event.key.keysym.sym == SDLK_RIGHT && !game.stop) {
				if (car.x + 20 + 45 < SCREEN_WIDTH)
				{
					car.x += 20;
				}
			}
			else if (event.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
			else if (event.key.keysym.sym == SDLK_UP && !game.stop) {

				if (car.speed + 1 <= 4.0)
					car.speed += 1;
			}
			else if (event.key.keysym.sym == SDLK_DOWN && !game.stop) {
				if (car.speed - 1 >= 0.0)
					car.speed -= 1;
			}
			else if (event.key.keysym.sym == SDLK_n) restart_game(game, time);
			else if (event.key.keysym.sym == SDLK_p) stop_game(car, time, game);
			break;
			//case SDL_KEYUP:
			//	//car.speed = 1.0;
			//	break;
		case SDL_QUIT:
			running = false;
			break;
		}
	}
}


void stop_game(car_t& car, gameTime_t& time, game_t& game) {
	game.stop == false ? game.stop = true : game.stop = false;
	time.t1 = SDL_GetTicks();
}

void restart_game(game_t& game, gameTime_t& time) {
	free(game.que.ptr);
	init_vector(&game.que);
	game.que.count = 0;
	free(game.road.ptr);
	init_vector(&game.road);
	game.road.count = 0;
	generate_start_road(game);
	game.score = 0;
	time.worldTime = 0;
	time.t1 = SDL_GetTicks();
	time.t2 = 0;
}


// Renderowanie samochodu
void renderCar(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture)
{
	SDL_Rect carRect;
	carRect.x = car.x;
	carRect.y = car.y;
	carRect.w = CAR_WIDTH;
	carRect.h = CAR_HEIGTH;
	SDL_RenderCopy(renderer, carTexture, nullptr, &carRect);
}
