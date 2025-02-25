// Waveshare 12956
// /296x128, 2.9inch E-Ink display module driver
#include QMK_KEYBOARD_H
#include <hardware/structs/systick.h>
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
0x0A,0x0,0x0,0x0,0x0,0x0,0x1,
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


static const uint8_t* g_image;


static void epd_gpio_config(void)
{
    // RP2040 CUP Clock: 125MHz, Waveshare e-paper SPI Clock: 20MHz
    // const uint EPD_SPI_DIVISOR = 125 / 20;
    spi_init();
    spi_start(EPD_CS_PIN, false, 0, 6);
    gpio_set_pin_input(EPD_BUSY_PIN);
    gpio_set_pin_output(EPD_RST_PIN);
    gpio_set_pin_output(EPD_DC_PIN);

    gpio_write_pin_high(EPD_RST_PIN);
    gpio_write_pin_high(EPD_DC_PIN);
}

static void epd_send_command(uint8_t cmd)
{
    if (gpio_read_pin(EPD_BUSY_PIN)){
        dprintf("Send command in spite of Busy: %02x\n", cmd);
    }

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
    enum dummy {
        SIZE_TEMP_BUFF = 64
    };
    uint count_temp_repeat = count / SIZE_TEMP_BUFF;
    uint count_temp_remain = count % SIZE_TEMP_BUFF;
    uint i;
    static uint8_t temp[SIZE_TEMP_BUFF];

    memset(temp, data, SIZE_TEMP_BUFF);

    gpio_write_pin_high(EPD_DC_PIN);
    for (i = 0; i < count_temp_repeat; i++){
        spi_transmit(temp, SIZE_TEMP_BUFF);
    }
    if (count_temp_remain > 0){
        spi_transmit(temp, count_temp_remain);
    }
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

static void epd_set_lut_by_host(const uint8_t* lut)
{
    epd_send_command(0x32);
    epd_send_data_block(lut, 153);
    epd_send_command(0x3f);
    epd_send_data(lut[153]);
    epd_send_command(0x03);
    epd_send_data(lut[154]);
    epd_send_command(0x04);
    epd_send_data(lut[155]);
    epd_send_data(lut[156]);
    epd_send_data(lut[157]);
    epd_send_command(0x2c);
    epd_send_data(lut[158]);
}

///////////////////////////////////////////////////////////////
// Control Sequencer Engine

typedef enum cseq_type {
    CSEQ_TYPE_END = 0,
    CSEQ_TYPE_FUNC_BV,
} cseq_type_t;

typedef bool (*cseq_func_bv_t)(void);

typedef union cseq_elem   cseq_elem_t;
typedef struct cseq_elem_header {
    cseq_type_t type;
} cseq_elem_header_t;
typedef struct cseq_elem_func_bv {
    cseq_type_t type;
    cseq_func_bv_t    func;
} cseq_elem_func_bv_t;
union cseq_elem {
    cseq_elem_header_t  header;
    cseq_elem_func_bv_t func_bv;
};

#define BEGIN_SEQ(NAME) const cseq_elem_t NAME[] = {
#define END_SEQ {.header={.type=CSEQ_TYPE_END}}};

#define SEQELEM_FUNC_BV(FUNC)  {.func_bv={.type=CSEQ_TYPE_FUNC_BV, .func=(FUNC)}}

static cseq_func_bv_t       func_cur;
static const cseq_elem_t*   seq_next;
static const cseq_elem_t*   pelem_cur;


static void cseq_engine_task(void)
{
    if (pelem_cur == NULL){
        if (seq_next == NULL){
            return;
        }
        pelem_cur = seq_next;
        seq_next = NULL;
    }
    if (func_cur != NULL){
        if (func_cur() == false){
            return;
        }
        func_cur = NULL;
    }
    switch (pelem_cur->header.type){
        case CSEQ_TYPE_END:
            pelem_cur = NULL;
            return;
        case CSEQ_TYPE_FUNC_BV:
            {
                const cseq_elem_func_bv_t* pefunc_bv = (const cseq_elem_func_bv_t*)pelem_cur;
                func_cur = pefunc_bv->func;
                break;
            }
    }
    pelem_cur++;
}

static void cseq_start(const cseq_elem_t* seq)
{
    seq_next = seq;
}

// Control Sequencer Engine
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Elements of Sequence

static uint32_t    tim_wait_start;
static uint32_t    ms_wait;

static bool seqelem_wait_until_idle(void)
{
    if (gpio_read_pin(EPD_BUSY_PIN)){
        return false;
    }
    return true;
}

static bool seqelem_wait_init_1of2(void)
{
    tim_wait_start = timer_read32();
    ms_wait = 1000;
    return true;
}

static bool seqelem_wait_init_2of2(void)
{
    if (timer_elapsed32(tim_wait_start) < ms_wait){
        return false;
    }
    return true;
}

static bool seqelem_rst_1of3(void)
{
    gpio_write_pin_low(EPD_RST_PIN);
    tim_wait_start = timer_read32();
    ms_wait = 10;
    return true;
}

static bool seqelem_rst_2of3(void)
{
    if (timer_elapsed32(tim_wait_start) < ms_wait){
        return false;
    }
    gpio_write_pin_high(EPD_RST_PIN);
    tim_wait_start = timer_read32();
    ms_wait = 10;
    return true;
}

static bool seqelem_rst_3of3(void)
{
    if (timer_elapsed32(tim_wait_start) < ms_wait){
        return false;
    }
    return true;
}

static bool seqelem_sw_reset_async(void)
{
    // soft reset
    epd_send_command(0x12);
    return true;
}

static bool seqelem_init(void)
{
    // Driver output control
    epd_send_command(0x01);
    epd_send_data(0x27);
    epd_send_data(0x01);
    epd_send_data(0x00);

    // Data entry mode
    epd_send_command(0x11);
    epd_send_data(0x03);

    // Display update control
    epd_send_command(0x21);
    epd_send_data(0x00);
    epd_send_data(0x80);

    return true;
}

static bool seqelem_init_fast(void)
{
    // Driver output control
    epd_send_command(0x01);
    epd_send_data(0x27);
    epd_send_data(0x01);
    epd_send_data(0x00);

    // Data entry mode
    epd_send_command(0x11);
    epd_send_data(0x03);

    // Select border waveform for VBD
    epd_send_command(0x3c);
    epd_send_data(0x05);

    // Display update control
    epd_send_command(0x21);
    epd_send_data(0x00);
    epd_send_data(0x80);

    return true;
}

static bool seqelem_set_lut_20_30(void)
{
    epd_set_lut_by_host(WS_20_30);
    return true;
}

static bool seqelem_set_lut_full(void)
{
    epd_set_lut_by_host(WF_FULL);
    return true;
}

static bool seqelem_set_lut_partial(void)
{
    epd_set_lut_by_host(WF_PARTIAL_2IN9);
    return true;
}

static bool seqelem_clear(void)
{
    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x24);
    epd_send_data_repeatedly(0xff, COUNT_IMAGE_BYTES);
    epd_send_command(0x26);
    epd_send_data_repeatedly(0xff, COUNT_IMAGE_BYTES);
    return true;
}

static bool seqelem_send_image(void)
{
    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x24);
    epd_send_data_block(g_image, COUNT_IMAGE_BYTES);
    return true;
}

static bool seqelem_init_partial_async(void)
{
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

    epd_send_command(0x3c);
    epd_send_data(0x80);

    epd_send_command(0x22);
    epd_send_data(0xc0);
    epd_send_command(0x20);
    return true;
}

static bool seqelem_turn_on_display_async(void)
{
    epd_send_command(0x22);
    epd_send_data(0xc7);
    epd_send_command(0x20);
    return true;
}

static bool seqelem_turn_on_display_partial_async(void)
{
    epd_send_command(0x22);
    epd_send_data(0x0f);
    epd_send_command(0x20);
    return true;
}

static bool seqelem_sleep_1of2(void)
{
    if (timer_elapsed32(tim_wait_start) < ms_wait){
        return false;
    }
    return true;
}

static bool seqelem_sleep_2of2(void)
{
    if (timer_elapsed32(tim_wait_start) < ms_wait){
        return false;
    }
    return true;
}

// Elements of Sequence
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Sequences

BEGIN_SEQ(seq_init)
    SEQELEM_FUNC_BV(seqelem_wait_init_1of2),
    SEQELEM_FUNC_BV(seqelem_wait_init_2of2),
    SEQELEM_FUNC_BV(seqelem_rst_1of3),
    SEQELEM_FUNC_BV(seqelem_rst_2of3),
    SEQELEM_FUNC_BV(seqelem_rst_3of3),
    SEQELEM_FUNC_BV(seqelem_sw_reset_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_init),
    SEQELEM_FUNC_BV(seqelem_set_lut_20_30),
    SEQELEM_FUNC_BV(seqelem_clear),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_sleep_1of2),
    SEQELEM_FUNC_BV(seqelem_sleep_2of2),
END_SEQ

BEGIN_SEQ(seq_show_image)
    SEQELEM_FUNC_BV(seqelem_rst_1of3),
    SEQELEM_FUNC_BV(seqelem_rst_2of3),
    SEQELEM_FUNC_BV(seqelem_rst_3of3),
    SEQELEM_FUNC_BV(seqelem_sw_reset_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_init_fast),
    SEQELEM_FUNC_BV(seqelem_set_lut_full),
    SEQELEM_FUNC_BV(seqelem_send_image),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_sleep_1of2),
    SEQELEM_FUNC_BV(seqelem_sleep_2of2),
END_SEQ

BEGIN_SEQ(seq_show_image_partial)
    SEQELEM_FUNC_BV(seqelem_rst_1of3),
    SEQELEM_FUNC_BV(seqelem_rst_2of3),
    SEQELEM_FUNC_BV(seqelem_rst_3of3),
    SEQELEM_FUNC_BV(seqelem_set_lut_partial),
    SEQELEM_FUNC_BV(seqelem_init_partial_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_send_image),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_partial_async),
    SEQELEM_FUNC_BV(seqelem_wait_until_idle),
    SEQELEM_FUNC_BV(seqelem_sleep_1of2),
    SEQELEM_FUNC_BV(seqelem_sleep_2of2),
END_SEQ

// Sequences
////////////////////////////////////////////////////

void epd_display_image(const uint8_t* image)
{
    g_image = image;
    cseq_start(seq_show_image);
}

void epd_display_image_partial(const uint8_t* image)
{
    g_image = image;
    cseq_start(seq_show_image_partial);
}

void epd_task_init(void)
{
    epd_gpio_config();
    cseq_start(seq_init);
}

void epd_task(void)
{
    cseq_engine_task();
}
