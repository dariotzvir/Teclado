#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/util.h"

int main()
{
    struct IO io;
    //stdio_init_all();
    gpio_init(io.led);
    gpio_set_dir(io.led ,GPIO_OUT);

    gpio_put(io.led, 1);
    sleep_ms(200);
    gpio_put(io.led, 0);
    sleep_ms(200);
    gpio_put(io.led, 1);
    sleep_ms(1000);

    for(int i=0; i<N; i++)
        gpio_set_dir(io.filas[i] ,GPIO_IN);
    
    for(int i=0; i<M; i++)
        gpio_set_dir(io.columnas[i] ,GPIO_IN);

    bool matriz[N][M] = {{0}};

    while(1)
    {
        gpio_put(io.led ,gpio_get(io.filas[0]));
        /*
        for(int i=0; i<N; i++)
        {
            for(int j=0; j<M; j++)
                printf("%d ", matriz[i][j]);
            printf("\n");
        }
        */
    }

    return 0;
}
