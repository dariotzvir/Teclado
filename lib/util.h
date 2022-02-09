#include "pico/stdlib.h"

//typedef unsigned int uint;
typedef unsigned char byte;

#define N 5
#define M 4
#define DEBOUNCE_PERIOD 50
#define BUFFER_LENGHT 32

struct IO
{
    const uint rows[N] = {11, 12, 13, 14, 15};
    const uint columns[M] = {0, 1, 2, 3};
    const uint led = 25;
};

struct KEY
{
    uint8_t modifier;
    uint8_t hid_key;
    unsigned long last_millis;
    bool pressed_flag;
    char ascii;
};
