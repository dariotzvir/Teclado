#include "pico/stdlib.h"

//typedef unsigned int uint;
typedef unsigned char byte;

#define N 5
#define M 4

struct IO
{
    const uint filas[N] = {11, 12, 13, 14, 15};
    const uint columnas[M] = {0, 1, 2, 3};
    const uint led = 25;
    const char keys[N][M] = {{'A','B','C','D'},{'A','B','C','D',},{'A','B','C','D'},{'A','B','C','D'},{'A','B','C','D'}}; 
};

