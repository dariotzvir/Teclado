#include "pico/stdlib.h"

//typedef unsigned int uint;
typedef unsigned char byte;

#define N 5
#define M 4

struct IO
{
    uint filas[N] = {11, 12, 13, 14, 15};
    uint columnas[M] = {0, 1, 2, 3};
    uint led = 25;
};