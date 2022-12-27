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


struct car_t;
struct gameTime_t;
struct fps_t;
struct game_t;
struct textures_t;
struct grass_t;


void free_textures(textures_t& textures);
void check_for_colision(car_t& car, game_t& game);
int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex);
void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window);
void load_charset(SDL_Surface*& charset);
void render_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture);
void events_handling(SDL_Event& event, car_t& car, game_t& game, gameTime_t& time);
void calculate_time(gameTime_t& time);
void render_legend(SDL_Surface* screen, SDL_Surface* charset, gameTime_t& time, fps_t& game_fps, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t game);
void cap_fps(fps_t& game_fps, car_t& car);
void generate_road_que(game_t& game);
void add_from_que_to_grass(game_t& game, car_t& car);
void generate_start_road(game_t& game);
void restart_game(game_t& game, gameTime_t& time, car_t& car);
void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car);
void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer);
void stop_game(car_t& car, gameTime_t& time, game_t& game);
void render_random_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture);
SDL_Texture* load_texture(char s[], SDL_Renderer* renderer);
void load_saves(char saves[10][128]);
void load_save_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[10][128]);
void load_save(game_t& game, gameTime_t& game_time, car_t& car, char file_name[]);
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void generate_random_car(textures_t textures, game_t* game);
void icrease_score(game_t& game, car_t& car);
void scroll_grass(game_t& game, car_t& player_car);
void clear_cars_outside_screen(game_t& game);
void update_cars_speed(game_t& game);
void render_cars(SDL_Renderer* renderer, textures_t& textures, game_t& game);
void manage_cars_position(game_t& game, car_t& player_car);
