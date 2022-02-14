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

void load_keys(struct KEY (*keys)[M]);
void matrix_read(struct IO io, struct KEY (*keys)[M]);
void hid_task(uint8_t modifier, uint8_t hid_key);

int main()
{

    board_init();
    tusb_init();
    stdio_init_all();

    struct KEY keys [N][M];
    load_keys(keys);


    gpio_init(SCROLL_LOCK_LED_GPIO);
    gpio_set_dir(SCROLL_LOCK_LED_GPIO, GPIO_OUT);
    gpio_init(CAPS_LOCK_LED_GPIO);
    gpio_set_dir(CAPS_LOCK_LED_GPIO, GPIO_OUT);
    gpio_init(NUM_LOCK_LED_GPIO);
    gpio_set_dir(NUM_LOCK_LED_GPIO, GPIO_OUT);

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
        tud_task();
        matrix_read(io, keys);

        //hid_task(ptr_buff, buffer);
    }

    return 0;
}

void load_keys(struct KEY (*keys)[M])
{
    const int hid_codes[N][M] = 
    {
        {HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3},
        {HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_KEYPAD_7, HID_KEY_KEYPAD_8},
        {HID_KEY_KEYPAD_COMMA, HID_KEY_KEYPAD_ADD, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_NUM_LOCK},
        {HID_KEY_SCROLL_LOCK, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_SYSREQ_ATTENTION, HID_KEY_BACKSLASH},
        {HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_9, HID_KEY_ESCAPE, HID_KEY_BACKSPACE}
    };
    const uint8_t hid_modifiers[N][M] = 
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    const uint8_t single_press[N][M] = 
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    const char asciis[N][M] = 
    {
        {'0', '1', '2', '3'},
        {'5', '6', '7', '8'},
        {',', '+', '-', 'n'},
        {'s', '*', 'y', '\\'},
        {'4', '9', 'e', 'd'}
    };
    
    for(int i=0; i<N; i++)
        for(int j=0; j<M; j++)
        {
            keys[i][j].hid_key = hid_codes[i][j];
            keys[i][j].modifier = hid_modifiers[i][j];
            keys[i][j].ascii = asciis[i][j];
            keys[i][j].single_press = single_press[i][j];
        }
}

void matrix_read(struct IO io, struct KEY (*keys)[M])
{
    char c = '\0';
    for(int i=0; i<M; i++)
    {
        gpio_put(io.columns[i], 1);

        for(int j=0; j<N; j++)
        {
            if(!keys[j][i].pressed_flag)
            {
                if(gpio_get(io.rows[j]) && !keys[j][i].debounce_flag)
                {
                    keys[j][i].last_millis_debounce = board_millis();
                    keys[j][i].debounce_flag = 1;
                } 
                if(keys[j][i].debounce_flag && board_millis() - keys[j][i].last_millis_debounce >= DEBOUNCE_PERIOD)
                {
                    if(gpio_get(io.rows[j])) 
                    {
                        hid_task(keys[j][i].modifier, keys[j][i].hid_key);
                        keys[j][i].pressed_flag = 1;
                        if(!keys[j][i].single_press) keys[j][i].last_millis_repetition = board_millis();
                    }
                    keys[j][i].debounce_flag = 0;
                }
            }
            else
            {
                if(!gpio_get(io.rows[j]) && !keys[j][i].debounce_flag)
                {
                    keys[j][i].last_millis_debounce = board_millis();
                    keys[j][i].debounce_flag = 1;
                } 
                if(keys[j][i].debounce_flag && board_millis() - keys[j][i].last_millis_debounce >= DEBOUNCE_PERIOD)
                {
                    if(!gpio_get(io.rows[j])) keys[j][i].pressed_flag = 0;
                    keys[j][i].debounce_flag = 0;
                }
                if(!keys[j][i].single_press && 
                board_millis() - keys[j][i].last_millis_repetition >= REPETITION_PERIOD)
                {
                    hid_task(keys[j][i].modifier, keys[j][i].hid_key);
                }
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
    tud_remote_wakeup();
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}


//--------------------------------------------------------------------+
// USB HID; i<0 && *buffer != 
static void send_hid_report(uint8_t report_id, uint8_t *keycode)
{
    if ( !tud_hid_ready() ) 
    {
        uart_puts(uart0, "return\n");
        return;
    }

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode); 
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(uint8_t modifier, uint8_t hid_key)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    uart_puts(uart0, "Task\n");

    if ( board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;
    
    uint8_t keycode [6] = {0};
    keycode [0] = hid_key;

    uart_puts(uart0, "keycode\n");

    // Remote wakeup
    if ( tud_suspended() && keycode[0] )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        uart_puts(uart0, "suspended\n");
        tud_remote_wakeup();
    }
    else
    {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_KEYBOARD, keycode);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    (void) instance;
    (void) len;
    static bool aux = 1;

    //uint8_t next_report_id = report[0] + 1;
    uint8_t keycode [6] = {0};
    keycode [0] = HID_KEY_KEYPAD_4;

    uart_puts(uart0, "complete\n");

    if(aux)
        send_hid_report(REPORT_ID_KEYBOARD, NULL);
    aux = !aux;
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

            if (kbd_leds & KEYBOARD_LED_NUMLOCK) gpio_put(NUM_LOCK_LED_GPIO, 1);
            else gpio_put(NUM_LOCK_LED_GPIO, 0);
            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) gpio_put(CAPS_LOCK_LED_GPIO, 1);
            else gpio_put(CAPS_LOCK_LED_GPIO, 0);
            if (kbd_leds & KEYBOARD_LED_SCROLLLOCK) gpio_put(SCROLL_LOCK_LED_GPIO, 1);
            else gpio_put(SCROLL_LOCK_LED_GPIO, 0);
        }
    }
}