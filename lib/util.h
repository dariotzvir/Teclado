#include "pico/stdlib.h"

//typedef unsigned int uint;
typedef unsigned char byte;

#define N 4
#define M 4

struct IO
{
    uint filas[N] = {1, 13, 15, 18};
    uint columnas[M] = {0, 12, 14, 19};
    uint led = 25;
};