#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/util.h"

#include "lib/tinyusb/src/tusb.h"
#include "lib/tinyusb/src/tusb_option.h"
#include "lib/usb_descriptors.h"

int main()
{
    struct IO io;
    //stdio_init_all();
    gpio_init(io.led);
    gpio_set_dir(io.led, GPIO_OUT);

    gpio_put(io.led, 1);
    sleep_ms(200);
    gpio_put(io.led, 0);
    sleep_ms(200);
    gpio_put(io.led, 1);
    sleep_ms(1000);

    for(int i=0; i<5; i++)
    {
        gpio_init(5+i);
        gpio_set_dir(5+i, GPIO_OUT);
        gpio_put(5+i, 1);
        sleep_ms(200);
        gpio_put(5+i, 0);
    }
    for(int i=0; i<N; i++)
    {
        gpio_init(io.filas[i]);
        gpio_set_dir(io.filas[i], GPIO_IN);
        gpio_pull_down(io.filas[i]);
    }
    for(int i=0; i<M; i++)
    {
        gpio_init(io.columnas[i]);
        gpio_set_dir(io.columnas[i], GPIO_OUT);
    }
    bool matriz[N][M] = {{0}};

    while(1)
    {
        for(int i=0; i<N; i++)
        {
            gpio_put(io.columnas[i], 1);

            for(int j=0; j<N; j++)
                gpio_put(5+j, gpio_get(io.filas[j]));

            gpio_put(io.columnas[i], 0);
        }
    }

    return 0;
}
