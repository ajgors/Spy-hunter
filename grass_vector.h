#include "main.h"

typedef struct {
    int allocated_size; // rozmiar zaalokowanego bufora
    int count;          // liczba elementów w wektorze
    grass_t* ptr;           // wskaŸnik do pocz¹tku bufora
} grass_vector_t;


void init_grass_vector(grass_vector_t* v);
void grass_reallocate(grass_vector_t* v, int reallocate_size);
void grass_push_back(grass_vector_t* v, grass_t val);
grass_t grass_pop_back(grass_vector_t* v);
void grass_add_to_front(grass_vector_t* v, grass_t val);

