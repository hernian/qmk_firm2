#pragma once

#include <stdint.h>

#define EPD_BUSY_PIN    GP20
#define EPD_MOSI_PIN    GP19
#define EPD_SCK_PIN     GP18
#define EPD_CS_PIN      GP10
#define EPD_DC_PIN      GP26
#define EPD_RST_PIN     GP27

#define EPD_WIDTH   128
#define EPD_HEIGHT  296
#define COUNT_IMAGE_BYTES  4736

void epd_init(void);
void epd_init_fast(void);
void epd_read_busy(void);
void epd_turn_in_display(void);
void epd_turn_in_display_partial(void);
void epd_clear(void);
void epd_clean_screen(void);
void epd_display(const uint8_t* image);
void epd_display_base(const uint8_t* image);
void epd_display_partial(const uint8_t* image);
void epd_sleep(void);
