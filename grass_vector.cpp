#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct grass_t {
    int y;
    int width;
};
//Struktura opisuj�ca wektor liczb ca�kowitych.
//j-ta kom�rka wektora w (0 <= j < w.count) jest dost�pna jest jako w.ptr[j].
typedef struct {
    int allocated_size; // rozmiar zaalokowanego bufora
    int count;          // liczba element�w w wektorze
    grass_t* ptr;           // wska�nik do pocz�tku bufora
} grass_vector_t;


// Inicjalizacja wektora *v. Pocz�tkowo tablica b�dzie mia�a rozmiar 1.
void init_grass_vector(grass_vector_t* v) {
    v->count = 0;
    v->allocated_size = 1;
    v->ptr = (grass_t*)malloc(v->allocated_size * sizeof(grass_t));
}

// Realokacja wektora *v, tak aby bufor mia� pojemno�� reallocate_size.
void grass_reallocate(grass_vector_t* v, int reallocate_size) {
    v->allocated_size = reallocate_size;
    grass_t* newPtr = (grass_t*)malloc(v->allocated_size * sizeof(grass_t));
    memcpy(newPtr, v->ptr, v->count * sizeof(grass_t));
    free(v->ptr);
    v->ptr = newPtr;
}


// Umieszczenie warto�ci val jako nowego (ko�cowego) elementu wektora *v.
void grass_push_back(grass_vector_t* v, grass_t val) {
    //Realokacja wymagana, gdy bufor nie jest w stanie pomie�ci� wi�cej element�w.
    if (v->count == v->allocated_size)
        grass_reallocate(v, 2 * v->allocated_size);
    v->ptr[v->count] = val;
    v->count++;
}


// Pobranie i usuni�cie warto�ci ko�cowego elementu wektora *v.
grass_t grass_pop_back(grass_vector_t* v) {
    v->count--;
    grass_t retv = v->ptr[v->count];

    //Realokacja realizowana, gdy rozmiar bufora jest ponad dwukrotnie
    //nadmiarowy w stosunku do liczby przechowywanych element�w.
    if (4 * v->count <= v->allocated_size)
        grass_reallocate(v, v->allocated_size / 2);
    return retv;
}
