// Waveshare 12956
// /296x128, 2.9inch E-Ink display module driver
#include QMK_KEYBOARD_H
#include "spi_master.h"
#include "epd.h"

#define EPD_BUSY_PIN    GP10
#define EPD_MOSI_PIN    SPI_MOSI_PIN
#define EPD_SCK_PIN     SPI_SCK_PIN
#define EPD_CS_PIN      GP20
#define EPD_DC_PIN      GP26
#define EPD_RST_PIN     GP27


static const uint8_t WS_20_30[159] =
{
0x80,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,
0x10,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x20,	0x0,	0x0,	0x0,
0x80,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,
0x10,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x20,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x14,	0x8,	0x0,	0x0,	0x0,	0x0,	0x1,
0xA,	0xA,	0x0,	0xA,	0xA,	0x0,	0x1,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x14,	0x8,	0x0,	0x1,	0x0,	0x0,	0x1,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x44,	0x44,	0x44,	0x44,	0x44,	0x44,	0x0,	0x0,	0x0,
0x22,	0x17,	0x41,	0x0,	0x32,	0x36
};

static const uint8_t WF_FULL[159] =
{
0x90,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L0	1.00S
0x60,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L1
0x90,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L2
0x60,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L3
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L4
0x19,	0x19,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group0
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group1
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group2
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group3
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group4
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group5
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group6
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group7
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group8
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group9
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group10
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group11
0x24,	0x42,	0x22,	0x22,	0x23,	0x32,	0x00,	0x00,	0x00,				//FR, XON
0x22,	0x17,	0x41,	0xAE,	0x32,	0x38,							//EOPT VGH VSH1 VSH2 VSL VCOM
};

static const uint8_t WF_PARTIAL_2IN9[159] =
{
0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0A,0x0,0x0,0x0,0x0,0x0,0x2,
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
0x22,0x17,0x41,0xB0,0x32,0x36,
};


static void epd_gpio_config(void)
{
    // RP2040 CUP Clock: 125MHz, Waveshare e-paper SPI Clock: 20MHz
    // const uint EPD_SPI_DIVISOR = 125 / 20;
    spi_init();
    spi_start(EPD_CS_PIN, false, 0, 6);
    gpio_set_pin_input(EPD_BUSY_PIN);
    gpio_set_pin_output(EPD_RST_PIN);
    gpio_set_pin_output(EPD_DC_PIN);
}


static void epd_send_command(uint8_t cmd)
{
    gpio_write_pin_low(EPD_DC_PIN);
    spi_transmit(&cmd, 1);
}

static void epd_send_data(uint8_t data)
{
    gpio_write_pin_high(EPD_DC_PIN);
    spi_transmit(&data, 1);
}

static void epd_send_data_block(const uint8_t* block, size_t len)
{
    gpio_write_pin_high(EPD_DC_PIN);
    spi_transmit(block, len);
}

static void epd_send_data_repeatedly(const uint8_t data, size_t count)
{
#if 0
    const uint SIZE_TEMP_BUFF = 16;
    uint count_temp_repeat = count / SIZE_TEMP_BUFF;
    uint count_temp_remain = count % SIZE_TEMP_BUFF;
    uint i;
    uint8_t temp[SIZE_TEMP_BUFF];

    memset(temp, data, SIZE_TEMP_BUFF);

    gpio_write_pin_high(EPD_DC_PIN);
    for (i = 0; i < count_temp_repeat; i++){
        spi_transmit(temp, SIZE_TEMP_BUFF);
    }
    if (count_temp_remain > 0){
        spi_transmit(temp, count_temp_remain);
    }
#endif
    uint i;
    gpio_write_pin_high(EPD_DC_PIN);
    for (i = 0; i < count; i++){
        spi_transmit(&data, 1);
    }
}

static void epd_reset(void)
{
    gpio_write_pin_high(EPD_RST_PIN);
    wait_ms(10);
    gpio_write_pin_low(EPD_RST_PIN);
    wait_ms(2);
    gpio_write_pin_high(EPD_RST_PIN);
    wait_ms(10);
}

static void epd_set_windows(uint x_start, uint y_start, uint x_end, uint y_end)
{
    epd_send_command(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    epd_send_data((uint8_t)(x_start >> 3));
    epd_send_data((uint8_t)(x_end >> 3));
    epd_send_command(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    epd_send_data((uint8_t)(y_start));
    epd_send_data((uint8_t)(y_start >> 8));
    epd_send_data((uint8_t)(y_end));
    epd_send_data((uint8_t)(y_end >> 8));
}

static void epd_set_cursor(uint x_start, uint y_start)
{
    epd_send_command(0x4e); // SET_RAM_X_ADDRESS_COUNTER
    epd_send_data((uint8_t)x_start);
    epd_send_command(0x4f); // SET_RAM_Y_ADDRESS_COUNTER
    epd_send_data((uint8_t)y_start);
    epd_send_data((uint8_t)(y_start >> 8));
}

static void epd_lut(const uint8_t* lut)
{
    epd_send_command(0x32);
    epd_send_data_block(lut, 153);
    epd_read_busy();
}

static void epd_lut_by_host(const uint8_t* lut)
{
    epd_lut(lut);
    epd_send_command(0x3f);
    epd_send_data(lut[153]);
    epd_send_command(0x03); // gate voltage
    epd_send_data(lut[154]);
    epd_send_command(0x04); // source voltage
    epd_send_data(lut[155]); // VSH
    epd_send_data(lut[156]); // VSH2
    epd_send_data(lut[157]); // VSL
    epd_send_command(0x2c); // VCOM
    epd_send_data(lut[158]);
}

void epd_init(void)
{
    epd_gpio_config();
    epd_reset();
    wait_ms(100);

    epd_read_busy();
    epd_send_command(0x12); // soft reset
    epd_read_busy();

    epd_send_command(0x01); // driver output control
    epd_send_data(0x27);
    epd_send_data(0x01);
    epd_send_data(0x00);

    epd_send_command(0x11); // data entry mode
    epd_send_data(0x03);

    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);

    epd_send_command(0x21); // display update control
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_set_cursor(0, 0);

    epd_lut_by_host(WS_20_30);
}

void epd_init_fast(void)
{
    epd_reset();
    wait_ms(100);

    epd_read_busy();
    epd_send_command(0x12); // soft reset
    epd_read_busy();

    epd_send_command(0x01); // Driver output control
    epd_send_data(0x27);
    epd_send_data(0x01);
    epd_send_data(0x00);

    epd_send_command(0x11); // data entry mode
    epd_send_data(0x03);

    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);

    epd_send_command(0x3c);
    epd_send_data(0x05);

    epd_send_command(0x21);
    epd_send_data(0x00);
    epd_send_data(0x80);

    epd_set_cursor(0, 0);
    epd_read_busy();

    epd_lut_by_host(WF_FULL);
}

void epd_read_busy(void)
{
    while (gpio_read_pin(EPD_BUSY_PIN))
    {
        wait_ms(50);
    }
    wait_ms(50);
}

void epd_turn_on_display(void)
{
    epd_send_command(0x22); // Display Update Control
    epd_send_data(0xC7);
    epd_send_command(0x20); // Activate Display Update Sequence
//    epd_read_busy();
}

void epd_turn_on_display_partial(void)
{
    epd_send_command(0x22); // Display Update Control
    epd_send_data(0x0f);
    epd_send_command(0x20); // Activate Display Update Sequence
    epd_read_busy();
}

void epd_clear(void)
{
    epd_send_command(0x24);
    epd_send_data_repeatedly(0xff, COUNT_IMAGE_BYTES);

    epd_send_command(0x26);
    epd_send_data_repeatedly(0xff, COUNT_IMAGE_BYTES);
}

void epd_clean_screen(void)
{
    epd_send_command(0x24);
    epd_send_data_repeatedly(0x00, COUNT_IMAGE_BYTES);

    epd_turn_on_display();

    epd_send_command(0x24);
    epd_send_data_repeatedly(0xff, COUNT_IMAGE_BYTES);

    epd_turn_on_display();
}

void epd_display(const uint8_t* image)
{
    epd_send_command(0x24);
    epd_send_data_block(image, COUNT_IMAGE_BYTES);

    epd_turn_on_display();
}

void epd_display_base(const uint8_t* image)
{
    epd_send_command(0x24);
    epd_send_data_block(image, COUNT_IMAGE_BYTES);

    epd_send_command(0x26);
    epd_send_data_block(image, COUNT_IMAGE_BYTES);

    epd_turn_on_display();
}

void epd_display_partial(const uint8_t* image)
{
    // reset
    gpio_write_pin_low(EPD_RST_PIN);
    wait_ms(1);
    gpio_write_pin_high(EPD_RST_PIN);
    wait_ms(2);

    epd_lut(WF_PARTIAL_2IN9);
    epd_send_command(0x37);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x40);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_send_command(0x3c); // BorderWaveform
    epd_send_data(0x80);

    epd_send_command(0x22);
    epd_send_data(0xc0);
    epd_send_command(0x20);
    epd_read_busy();

    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x24);
    epd_send_data_block(image, COUNT_IMAGE_BYTES);

    epd_turn_on_display_partial();
}

void epd_sleep(void)
{
    epd_send_command(0x10); // enter deep sleep
    epd_send_data(0x01);
    wait_ms(100);
}


static const uint8_t* g_image = NULL;
static int g_state = 0;
static uint g_time_mark;
static uint g_time_delay;


static bool epd_every_sec(uint32_t* tim_pre)
{
    uint32_t tim_cur = timer_read();

    if (tim_cur - *tim_pre < 1000){
        return false;
    }
    *tim_pre = tim_cur;
    return true;
}

static void epd_delay_async(uint delay)
{
    g_time_delay = delay;
    g_time_mark = timer_read();
}

static bool epd_delay_wait(void)
{
    return timer_elapsed(g_time_mark) >= g_time_delay;
}

bool epd_busy_wait(void)
{
    return !gpio_read_pin(EPD_BUSY_PIN);
}

void epd_init_req(void)
{
    g_state = 100;
}

void epd_init_1_async(void)
{
    dprint("[epd_init_1_async]start\n");
    epd_gpio_config();
    epd_reset();
    epd_delay_async(100);
}

bool epd_init_1_wait(void)
{
    return epd_delay_wait();
}

bool epd_init_2_wait(void)
{
    static uint32_t tim_pre;
    uint busy = gpio_read_pin(EPD_BUSY_PIN);
    if (epd_every_sec(&tim_pre)){
        dprintf("[epd_init_2_wait]busy %u\n", busy);
    }
    return !busy;
}

void epd_init_3_async(void)
{
    dprint("[epd_init_3_async]start\n");
    epd_send_command(0x12); // soft reset
}

bool epd_init_3_wait(void)
{
    static uint32_t tim_pre;
    uint busy = gpio_read_pin(EPD_BUSY_PIN);
    if (epd_every_sec(&tim_pre)){
        dprintf("[epd_init_3_wait]busy %u\n", busy);
    }
    return !busy;
}

void epd_init_4(void)
{
    dprint("[epd_init_4]start\n");
    epd_send_command(0x01); // driver output control
    epd_send_data(0x27);
    epd_send_data(0x01);
    epd_send_data(0x00);

    epd_send_command(0x11); // data entry mode
    epd_send_data(0x03);

    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);

    epd_send_command(0x21); // display update control
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_set_cursor(0, 0);

    epd_lut_by_host(WS_20_30);

    dprint("[epd_init_4]end\n");
}

static void epd_set_image(const uint8_t* image)
{
    epd_send_command(0x24);
    epd_send_data_block(image, COUNT_IMAGE_BYTES);
}

static void epd_turn_on_display_async(void)
{
    epd_send_command(0x22); // Display Update Control
    epd_send_data(0xC7);
    epd_send_command(0x20); // Activate Display Update Sequence
}

static bool epd_turn_on_display_wait(void)
{
    return epd_busy_wait();
}

static void epd_sleep_async(void)
{
    epd_send_command(0x10); // enter deep sleep
    epd_send_data(0x01);
    epd_delay_async(100);
}

static bool epd_sleep_wait(void)
{
    return epd_delay_wait();
}

void epd_display_image_req(const uint8_t* image)
{
    g_image = image;
}


void epd_task(void)
{
    switch (g_state) {
        case 0:
            if (g_image == NULL){
                break;
            }
            dprint("[epd_task] changing image start\n");
            epd_init_fast();
            g_state++;
            break;
        case 1:
            dprint("[epd_task] changing image #1\n");
            epd_set_image(g_image);
            g_image = NULL;
            g_state++;
            break;
        case 2:
            dprint("[epd_task] changing image #2\n");
            epd_turn_on_display_async();
            g_state++;
            break;
        case 3:
            if (!epd_turn_on_display_wait()){
                break;
            }
            dprint("[epd_task] changing image #3\n");
            g_state++;
            break;
        case 4:
            dprint("[epd_task] changing image #4\n");
            epd_sleep_async();
            g_state++;
            break;
        case 5:
            if (!epd_sleep_wait()){
                break;
            }
            dprint("[epd_task] changing image end\n");
            g_state = 0;
            break;
        case 100:
            dprint("[epd_task] 100\n");
            epd_init_1_async();
            g_state++;
            break;
        case 101:
            if (!epd_init_1_wait()){
                break;
            }
            dprint("[epd_task] 101\n");
            g_state++;
        case 102:
            // epd_init_2_async()は無い
            if (!epd_init_2_wait()){
                break;
            }
            dprint("[epd_task] 102\n");
            epd_init_3_async();
            g_state++;
            break;
        case 103:
            if (!epd_init_3_wait()){
                break;
            }
            dprint("[epd_task] 103\n");
            epd_init_4();
            g_state = 0;
            break;
    }
}
