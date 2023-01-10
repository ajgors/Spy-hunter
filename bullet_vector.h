#include "main.h"

typedef struct {
    int allocated_size; 
    int count;          
    bullet_t* ptr;      
} bullet_vector_t;


void bullet_vector_delete(bullet_vector_t* v, int index);
bullet_t bullet_pop_back(bullet_vector_t* v);
void bullet_push_back(bullet_vector_t* v, bullet_t val);
void bullet_reallocate(bullet_vector_t* v, int reallocate_size);
void init_bullet_vector(bullet_vector_t* v);
