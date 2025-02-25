#include QMK_KEYBOARD_H
#include <print.h>
#include "ch.h"
#include "epd.h"

extern const uint8_t Image128x296[];

static uint8_t g_myBitmap[COUNT_IMAGE_BYTES];


void init_my_bitmap(void)
{
    const uint COUNT_BYTES_IN_LINE = (EPD_WIDTH + 7) / 8;
    uint y;
    uint i;
    uint i_border;
    uint8_t* line;

    i_border = 0;
    for (y = 0; y < EPD_HEIGHT; y++)
    {
        line = &g_myBitmap[y * COUNT_BYTES_IN_LINE];
        for (i = 0; i < i_border; i++)
        {
            line[i] = 0x00;
        }
        for (i = i_border; i < COUNT_BYTES_IN_LINE; i++)
        {
            line[i] = 0xff;
        }
        if (y % 8 == 7){
            i_border++;
            if (i_border > COUNT_BYTES_IN_LINE)
            {
                i_border = 0;
            }
        }
    }
}


void init_ichimatsu_bitmap(void)
{
    const uint COUNT_BYTES_IN_LINE = (EPD_WIDTH + 7) / 8;
    uint y;
    uint i;
    uint8_t pixels;
    uint8_t pixels_temp;
    uint8_t* line;

    pixels = 0xff;
    for (y = 0; y < EPD_HEIGHT; y++)
    {
        line = &g_myBitmap[y * COUNT_BYTES_IN_LINE];
        pixels_temp = pixels;
        for (i = 0; i < COUNT_BYTES_IN_LINE; i++)
        {
            line[i] = pixels_temp;
            pixels_temp = ~pixels_temp;
        }
        if (y % 8 == 7){
            pixels = ~pixels;
        }
    }
}


void keyboard_post_init_kb(void)
{
    debug_enable = true;
    debug_matrix = true;
    dprint("[keyboard_post_init_kb]enter");

    init_my_bitmap();

    epd_task_init();
#if 0
    epd_init();
    epd_clear();
    epd_sleep();
#endif
    keyboard_post_init_user();
    dprint("[keyboard_post_init_kb]leave");
}

void housekeeping_task_kb(void)
{
    epd_task();
    housekeeping_task_user();
}


bool process_record_kb(uint16_t keycode, keyrecord_t *record)
{
    (void)Image128x296;
    if (record->event.pressed){
        dprintf("key down: %04X\n", keycode);
        if (keycode == 0x14){
            epd_display_image_partial(Image128x296);
        }
    }
    else{
        if (keycode == 0x14){
            init_ichimatsu_bitmap();
            epd_display_image(g_myBitmap);
        }
    }

    return process_record_user(keycode, record);
}



