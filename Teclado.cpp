#include <stdio.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "lib/util.h"

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void matrix_read(struct IO io, struct KEY (*keys)[M]);
void hid_task(uint8_t *ptr_buff, uint8_t *buffer);

int main()
{
    uint8_t buffer [BUFFER_LENGHT] = {0};
    uint8_t *ptr_buff;

    board_init();
    tusb_init();
    stdio_init_all();

    const char a[N][M] = {{'A','B','C','D'},{'E','F','G','H'},{'I','J','K','L'},{'M','N','O','P'},{'Q','R','S','T'}}; 
    struct KEY keys [N][M];
    for(int i=0; i<N; i++)
        for(int j=0; j<M; j++)
        {
            keys[i][j].ascci_key = a[i][j];
            keys[i][j].pressed_flag = 0;
        }

    uart_init(uart0, 115200);
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);

    struct IO io;
    gpio_init(io.led);
    gpio_set_dir(io.led, GPIO_OUT);

    gpio_init(18);
    gpio_set_dir(18, GPIO_IN);
    gpio_pull_up(18);

    for(int i=0; i<N; i++)
    {
        gpio_init(io.rows[i]);
        gpio_set_dir(io.rows[i], GPIO_IN);
        gpio_pull_down(io.rows[i]);
    }
    for(int i=0; i<M; i++)
    {
        gpio_init(io.columns[i]);
        gpio_set_dir(io.columns[i], GPIO_OUT);
    }
    while(1)
    {
        //uart_puts(uart0, "Hello world!");
        tud_task(); // tinyusb device task
        
        matrix_read(io, keys);

        hid_task();
    }

    return 0;
}

void matrix_read(struct IO io, struct KEY (*keys)[M])
{
    char c = '\0';
    for(int i=0; i<M; i++)
    {
        gpio_put(io.columns[i], 1);

        for(int j=0; j<N; j++)
        {
            if(gpio_get(io.rows[j]) && !keys[j][i].pressed_flag)
            {
                keys[j][i].last_millis = board_millis();
                keys[j][i].pressed_flag = 1;
            } 
            if(keys[j][i].pressed_flag && board_millis() - keys[j][i].last_millis >= DEBOUNCE_PERIOD)
            {
                if(gpio_get(io.rows[j]))
                {
                    c = keys[i][j].ascci_key;

                    char a[3] = {0};
                    a[0] = c;
                    a[1] = '\n';

                    

                    uart_puts(uart0, a);
                }
                keys[j][i].pressed_flag = 0;
            }
        }    
        gpio_put(io.columns[i], 0);
    }
}



// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    //tud_remote_wakeup();
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint8_t *keycode)
{
    // skip if hid is not ready yet
    if ( !tud_hid_ready() ) return;

    //btn = gpio_get(18);
    // use to avoid send multiple consecutive zero report for keyboard
    //static bool has_keyboard_key = false;
    //uart_puts(uart0, "Keyboard\n");

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode); 
/*
    if ( btn )
    {
        //uart_puts(uart0, "if\n");

        //uint8_t keycode[6] = { 0 };
        //keycode[0] = HID_KEY_A;
        //keycode[1] = HID_KEY_B;
        //keycode[2] = HID_KEY_C;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode); 
        //sleep_ms(1000);
        //tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = true;
    }
    else
    {
        //uart_puts(uart0, "else\n");
        // send empty key report if previously has key pressed
        if (has_keyboard_key) 
        {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            //uart_puts(uart0, "has_keyboard_key\n");
        }
        has_keyboard_key = false;
    }*/
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(uint8_t *ptr_buff, uint8_t *buffer)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();
    
    uint8_t keycode [6] = {0};
    for(int i=5; i<0 && *buffer != '\0'; i--)
    {
        keycode[i] = *ptr_buff;

    }

    // Remote wakeup
    if ( tud_suspended() && btn )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_KEYBOARD, btn);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    (void) instance;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    send_hid_report(next_report_id, gpio_get(18));
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD)
        {
            // bufsize should be (at least) 1
            if ( bufsize < 1 ) return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_NUMLOCK) board_led_write(true);
            else board_led_write(false);
        }
    }
}
