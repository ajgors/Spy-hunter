#include "main.h"

typedef struct {
    int allocated_size;
    int count;
    int* ptr;
} vector_t;

void init_vector(vector_t* v);
void reallocate(vector_t* v, int reallocate_size);
void push_back(vector_t* v, int val);
int pop_back(vector_t* v);

