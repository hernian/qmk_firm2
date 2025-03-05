#include QMK_KEYBOARD_H
#include <print.h>
#include "ch.h"
#include "epd_ws1in54_V2.h"
#include "generated/img_epaper_200x200_navigation.h"
#include "generated/img_epaper_200x200_fusion360.h"
#include "generated/img_epaper_200x200_illustrator.h"
#include "generated/img_epaper_200x200_kicad.h"
#include "generated/img_epaper_200x200_numkey.h"
#include "generated/img_epaper_200x200_selectmode.h"


static const uint8_t* const img_list[] = {
    img_epaper_200x200_navigation,
    img_epaper_200x200_fusion360,
    img_epaper_200x200_illustrator,
    img_epaper_200x200_kicad,
    img_epaper_200x200_numkey,
};
static const int COUNT_IMG_LIST = sizeof(img_list) / sizeof(img_list[0]);
static int idx_img;


void keyboard_post_init_kb(void)
{
    debug_enable = true;
    debug_matrix = true;

    epd_task_init(img_list[idx_img]);
    keyboard_post_init_user();
}

void housekeeping_task_kb(void)
{
    epd_task();
    housekeeping_task_user();
}


bool process_record_kb(uint16_t keycode, keyrecord_t *record)
{
    if (record->event.pressed){
        if (keycode == 0x14){
            epd_display_image_partial(img_epaper_200x200_selectmode);
        }
    }
    else{
        if (keycode == 0x14){
            idx_img++;
            if (idx_img >= COUNT_IMG_LIST){
                idx_img = 0;
            }
            dprintf("show image. idx_img:%u\n", idx_img);
            epd_display_image_partial(img_list[idx_img]);
        }
    }
    return process_record_user(keycode, record);
}



