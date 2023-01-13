#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct car_t {
    int x;
    int y;
    double speed;
    int crashed_at;
    int type;
    bool on_fire;
};

typedef struct {
    int allocated_size;     // rozmiar zaalokowanego bufora
    int count;              // liczba elementów w wektorze
    car_t* ptr;             // wskaŸnik do pocz¹tku bufora
} car_vector_t;


// Inicjalizacja wektora *v. Pocz¹tkowo tablica bêdzie mia³a rozmiar 1.
void init_car_vector(car_vector_t* v) {
    v->count = 0;
    v->allocated_size = 1;
    v->ptr = (car_t*)malloc(v->allocated_size * sizeof(car_t));
}

// Realokacja wektora *v, tak aby bufor mia³ pojemnoœæ reallocate_size.
void car_reallocate(car_vector_t* v, int reallocate_size) {
    v->allocated_size = reallocate_size;
    car_t* newPtr = (car_t*)malloc(v->allocated_size * sizeof(car_t));
    memcpy(newPtr, v->ptr, v->count * sizeof(car_t));
    free(v->ptr);
    v->ptr = newPtr;
}


// Umieszczenie wartoœci val jako nowego (koñcowego) elementu wektora *v.
void car_push_back(car_vector_t* v, car_t val) {
    //Realokacja wymagana, gdy bufor nie jest w stanie pomieœciæ wiêcej elementów.
    if (v->count == v->allocated_size)
        car_reallocate(v, 2 * v->allocated_size);
    v->ptr[v->count] = val;
    v->count++;
}


// Pobranie i usuniêcie wartoœci koñcowego elementu wektora *v.
car_t car_pop_back(car_vector_t* v) {
    v->count--;
    car_t retv = v->ptr[v->count];

    //Realokacja realizowana, gdy rozmiar bufora jest ponad dwukrotnie
    //nadmiarowy w stosunku do liczby przechowywanych elementów.
    if (4 * v->count <= v->allocated_size)
        car_reallocate(v, v->allocated_size / 2);
    return retv;
}

void car_vector_delete(car_vector_t* v, int index){
    
    if (index < 0 || index >= v->count)
        return;

    v->ptr[index] = {0};

    for (int i = index; i < v->count - 1; i++) {
        v->ptr[i] = v->ptr[i + 1];
        v->ptr[i + 1] = {0};
    }

    v->count--;

    if (v->count > 0 && v->count == v->allocated_size / 4)
        car_reallocate(v, v->allocated_size / 2);
}