#pragma once

#include <stdint.h>


#define EPD_WIDTH   128
#define EPD_HEIGHT  296
#define COUNT_IMAGE_BYTES  4736

void epd_display_image(const uint8_t* image);
void epd_display_image_partial(const uint8_t* image);

void epd_task_init(void);
void epd_task(void);
