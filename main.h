#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include <vector>
using namespace std;


#define SCREEN_WIDTH 650
#define SCREEN_HEIGHT 480
#define FPS 60
#define LEGEND_HEIGHT 50
#define GRASS_HEIGHT 32
#define GRASS_WIDTH 25
#define CAR_HEIGTH 74
#define CAR_WIDTH 36
#define CAR_X (SCREEN_WIDTH / 2) - (CAR_WIDTH / 2)
#define CAR_Y (SCREEN_HEIGHT / 2) + (LEGEND_HEIGHT)
#define ERROR 0
#define SUCCESS 1
#define CAR_MOVE_PIXELS 25
#define IMPLEMENTED_WIDTH 80
#define IMPLEMENTED_HEIGHT 20
#define MAX_GRASS_WIDTH 8*GRASS_WIDTH
#define MIN_GRASS_WIDTH GRASS_WIDTH
#define NUMBER_OF_GRASS_TXT (SCREEN_HEIGHT / GRASS_HEIGHT)
#define CARS_NUMBER 10
#define START_LIVES 3
#define FIRE_WIDTH 100
#define FIRE_HEIGTH 100
#define HEART_WIDTH 20
#define HEART_HEIGTH 20
#define BULLET_WIDTH 1
#define BULLET_HEIGTH 10
#define CAR_RESPAWN_TIME 1500	// in Miliseconds
#define INF_LIVES_TIME 60		// in Seconds
#define SAVES_NUMBER 10
#define BULLET_SPEED 7
#define SPEED_INCREMENT 4
#define MAX_SPEED 4 * SPEED_INCREMENT
#define START_SPEED 2 * SPEED_INCREMENT
#define TEXT_CENTER screen->w / 2 - strlen(text) * 8 / 2
#define GRASS_SPEED 4
#define MAX_CARS 14
#define OUT_OF_SCREEN -1
#define NORMAL 0
#define HOSTILE 1

struct car_t;
struct gameTime_t;
struct fps_t;
struct game_t;
struct textures_t;
struct grass_t;
struct bullet_t;
struct colors_t;


void pick_up_heart(game_t& game, car_t& player_car);
void generate_random_heart(game_t& game, gameTime_t& time);
void free_textures(textures_t& textures);
void check_for_grass_colision(car_t& car, game_t& game, gameTime_t& time);
int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex);
void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window);
void load_charset(SDL_Surface*& charset);
void render_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture);
void events_handling(SDL_Event& event, car_t& car, game_t& game, gameTime_t& time);
void calculate_time(gameTime_t& time);
void render_legend(SDL_Surface* screen, SDL_Surface* charset, gameTime_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game, colors_t& colors);
void cap_fps(fps_t& game_fps, car_t& car);
void generate_road_que(game_t& game);
void generate_start_road(game_t& game);
void restart_game(game_t& game, gameTime_t& time, car_t& car);
void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car);
void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer);
void stop_game(car_t& car, gameTime_t& time, game_t& game);
SDL_Texture* load_texture(char s[], SDL_Renderer* renderer);
void load_saves(char saves[10][128]);
void show_saves_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[10][128]);
void load_save(game_t& game, gameTime_t& game_time, car_t& car, char file_name[]);
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void generate_random_car(textures_t textures, game_t* game, car_t& player_car);
void icrease_score(game_t& game, car_t& car);
void scroll_grass(game_t& game, car_t& player_car);
void remove_cars_outside_screen(game_t& game);
void update_cars_speed(game_t& game);
void render_cars(SDL_Renderer* renderer, textures_t& textures, game_t& game, car_t& player_car);
void manage_cars_position(game_t& game, car_t& player_car, gameTime_t& time);
void render_bullet(SDL_Renderer* renderer, game_t& game, SDL_Texture* bullet_texture);
void render_fire(SDL_Renderer* renderer, game_t& game, SDL_Texture* fire);
void render_heart(SDL_Renderer* renderer, game_t& game, SDL_Texture* heart, car_t& player_car);
void move_bullets(game_t& game);
void init_colors(colors_t& colors, SDL_Surface* screen);
void show_gameover_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, game_t& game);
void show_pause_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer);
void load_picked_save(SDL_Event& event, game_t& game, gameTime_t& time, car_t& player_car, char  saves[10][128]);
void move_hostile_car_to_player(game_t& game, car_t& player_car);
