#include "main.h"

typedef struct {
    int allocated_size;
    int count;
    car_t* ptr;
} car_vector_t;

void init_car_vector(car_vector_t* v);
void car_reallocate(car_vector_t* v, car_t reallocate_size);
void car_push_back(car_vector_t* v, car_t val);
car_t car_pop_back(car_vector_t* v);
void car_vector_delete(car_vector_t* v, int index);

