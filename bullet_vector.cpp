#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"


struct bullet_t {
    int y = CAR_Y;
    int x = CAR_X;
    int time = 0;
};

//Struktura opisuj�ca wektor liczb ca�kowitych.
//j-ta kom�rka wektora w (0 <= j < w.count) jest dost�pna jest jako w.ptr[j].
typedef struct {
    int allocated_size; // rozmiar zaalokowanego bufora
    int count;          // liczba element�w w wektorze
    bullet_t* ptr;           // wska�nik do pocz�tku bufora
} bullet_vector_t;


// Inicjalizacja wektora *v. Pocz�tkowo tablica b�dzie mia�a rozmiar 1.
void init_bullet_vector(bullet_vector_t* v) {
    v->count = 0;
    v->allocated_size = 1;
    v->ptr = (bullet_t*)malloc(v->allocated_size * sizeof(bullet_t));
}

// Realokacja wektora *v, tak aby bufor mia� pojemno�� reallocate_size.
void bullet_reallocate(bullet_vector_t* v, int reallocate_size) {
    v->allocated_size = reallocate_size;
    bullet_t* newPtr = (bullet_t*)malloc(v->allocated_size * sizeof(bullet_t));
    memcpy(newPtr, v->ptr, v->count * sizeof(bullet_t));
    free(v->ptr);
    v->ptr = newPtr;
}


// Umieszczenie warto�ci val jako nowego (ko�cowego) elementu wektora *v.
void bullet_push_back(bullet_vector_t* v, bullet_t val) {
    //Realokacja wymagana, gdy bufor nie jest w stanie pomie�ci� wi�cej element�w.
    if (v->count == v->allocated_size)
        bullet_reallocate(v, 2 * v->allocated_size);
    v->ptr[v->count] = val;
    v->count++;
}


// Pobranie i usuni�cie warto�ci ko�cowego elementu wektora *v.
bullet_t bullet_pop_back(bullet_vector_t* v) {
    v->count--;
    bullet_t retv = v->ptr[v->count];

    //Realokacja realizowana, gdy rozmiar bufora jest ponad dwukrotnie
    //nadmiarowy w stosunku do liczby przechowywanych element�w.
    if (4 * v->count <= v->allocated_size)
        bullet_reallocate(v, v->allocated_size / 2);
    return retv;
}

void bullet_vector_delete(bullet_vector_t* v, int index)
{
    if (index < 0 || index >= v->count)
        return;

    v->ptr[index] = { 0 };

    for (int i = index; i < v->count - 1; i++) {
        v->ptr[i] = v->ptr[i + 1];
        v->ptr[i + 1] = { 0 };
    }

    v->count--;

    if (v->count > 0 && v->count == v->allocated_size / 4)
        bullet_reallocate(v, v->allocated_size / 2);
}