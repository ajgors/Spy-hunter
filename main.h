#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include <vector>
using namespace std;

//width and height constants
#define IMPLEMENTED_WIDTH 105
#define IMPLEMENTED_HEIGHT 20
#define MAX_GRASS_WIDTH 8*GRASS_WIDTH
#define MIN_GRASS_WIDTH GRASS_WIDTH
#define NUMBER_OF_GRASS_TXT (SCREEN_HEIGHT / GRASS_HEIGHT)
#define FIRE_WIDTH 100
#define FIRE_HEIGTH 100
#define ITEM_WIDTH 20
#define ITEM_HEIGTH 20
#define BULLET_WIDTH 1
#define BULLET_HEIGTH 10
#define LEGEND_HEIGHT 50
#define GRASS_HEIGHT 32
#define GRASS_WIDTH 25
#define CAR_HEIGTH 74
#define CAR_WIDTH 36
#define SCREEN_WIDTH 650
#define SCREEN_HEIGHT 480
//time constants
#define CAR_RESPAWN_TIME 1500	// in Miliseconds
#define INF_LIVES_TIME 30		// in Seconds
#define HALT_TIME 3000			// in Miliseconds
#define POWER_UP_TIME 10000		// in Miliseconds
#define BULLET_DELAY 350		// in Miliseconds
//coordinates constants
#define CAR_X (SCREEN_WIDTH / 2) - (CAR_WIDTH / 2)
#define CAR_Y (SCREEN_HEIGHT / 2) + (LEGEND_HEIGHT)
#define LEGEND_X 0
#define LEGEND_Y 0
#define SAVES_Y 40
#define PAUSE_SCREEN_Y 50
#define LIST_SCREEN_Y 34
#define NEXT_LINE_Y 16
#define GAME_OVER_Y 50
#define TEXT_CENTER screen->w / 2 - strlen(text) * 8 / 2
//probability constants
#define ITEM_PROB 1000			//Random item probability
#define CAR_PROB 100			//Random car probability
#define HOSTILE_CAR_MOVE_PROB 2 //Hostile car move probability
//additional constants
#define FPS 60
#define ERROR 0
#define SUCCESS 1
#define CAR_MOVE_PIXELS 3
#define CARS_NUMBER 10			//Max number of random cars
#define START_LIVES 3
#define SAVES_NUMBER 10
#define BULLET_SPEED 7
#define SPEED_INCREMENT 1
#define MAX_SPEED 12
#define START_SPEED 4
#define MAX_CARS 6
#define NORMAL 0				//Random car type - Normal
#define HOSTILE 1				//Random car type - Hostile
#define SCORES_FILE "scores.txt"
#define SAVES_FILE "saves.txt"
#define FILE_ERR "ERROR WHILE OPENING FILE"
#define SIZEOF_SAVE_STRING 21

struct car_t;
struct game_time_t;
struct fps_t;
struct game_t;
struct textures_t;
struct grass_t;
struct bullet_t;
struct colors_t;
struct item_t;
struct scores_t;

bool pick_up_item(car_t& player_car, item_t& item);
void free_textures(textures_t& textures);
void check_for_grass_colision(car_t& player_car, game_t& game, game_time_t& time);
int init(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex);
void free_memory(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window);
bool load_charset(SDL_Surface*& charset);
void render_car(car_t& car, SDL_Renderer* renderer, SDL_Texture* carTexture);
void events_handling(SDL_Event& event, car_t& car, game_t& game, game_time_t& time, scores_t& saved_scores);
void calculate_time(game_time_t& time);
void render_legend(SDL_Surface* screen, SDL_Surface* charset, game_time_t& time, SDL_Renderer* renderer, SDL_Texture* scrtex, game_t *game, colors_t& colors, car_t& player_car);
void cap_fps(fps_t& game_fps, car_t& car);
void generate_grass_que(game_t& game);
void generate_start_grass(game_t& game);
void restart_game(game_t& game, game_time_t& time, car_t& car);
void render_grass(game_t& game, SDL_Renderer* renderer, SDL_Texture* roadTexture, car_t& car);
void render_implemented(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, colors_t& colors);
void stop_game(car_t& car, game_time_t& time, game_t& game);
SDL_Texture* load_texture(char s[], SDL_Renderer* renderer, vector<SDL_Texture*>& txt_p);
void get_saves_name(char saves[SAVES_NUMBER][128]);
void show_saves_screen(SDL_Surface* screen, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Event& event, char saves[SAVES_NUMBER][128], colors_t& colors);
void load_save(game_t& game, game_time_t& game_time, car_t& car, char file_name[]);
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void generate_random_car(textures_t textures, game_t* game, car_t& player_car);
void icrease_score(game_t& game, car_t& car);
void scroll_grass(game_t& game, car_t& player_car);
void remove_cars_outside_screen(game_t& game);
void render_cars(SDL_Renderer* renderer, textures_t& textures, game_t& game, car_t& player_car);
void manage_cars_position(game_t& game, car_t& player_car, game_time_t& time);
void render_bullet(SDL_Renderer* renderer, game_t& game, SDL_Texture* bullet_texture);
void render_fire(SDL_Renderer* renderer, car_t& player_car, SDL_Texture* fire);
void render_item(SDL_Renderer* renderer, item_t& item, SDL_Texture* heart_txt, car_t& player_car);
void move_bullets(game_t& game);
void init_colors(colors_t& colors, SDL_Surface* screen);
void show_gameover_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, game_t& game);
void show_pause_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer);
void load_picked_save(SDL_Event& event, game_t& game, game_time_t& time, car_t& player_car, char  saves[SAVES_NUMBER][128]);
void move_hostile_car_to_player(game_t& game, car_t& player_car);
void destroy_car(car_t& player_car, game_t& game, game_time_t& time);
void sort_by_points(scores_t& saved_scores);
void sort_by_time(scores_t& saved_scores);
void show_list_screen(SDL_Surface* screen, colors_t& colors, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, scores_t& saved_scores, game_t& game);
void load_scores_list(scores_t& saved_scores);
void save_score(game_t& game, game_time_t& time, scores_t& saved_scores);
int load_scores_size();
void pick_up_power_up(game_t& game, car_t& player_car);
void pick_up_heart(game_t& game, car_t& player_car);
void generate_random_item(item_t& item, game_time_t& time, game_t& game);
void calculate_power_up_time_left(game_t& game);
void generate_random_power_up(item_t& power_up, game_time_t& time, game_t& game);
void generate_random_heart(item_t& heart, game_time_t& time, game_t& game);
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor);
bool load_textures(textures_t& textures, SDL_Renderer* renderer);
void init_game(SDL_Window*& window, SDL_Renderer*& renderer, SDL_Surface*& screen, SDL_Texture*& scrtex, SDL_Surface*& charset, textures_t& textures);
int load_size_from_file(char file_name[128]);