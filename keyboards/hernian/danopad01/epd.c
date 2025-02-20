// Waveshare 12956
// /296x128, 2.9inch E-Ink display module driver
#include "gpio.h"
#include "wait.h"
#include "epd.h"


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
    gpio_set_pin_input(EPD_BUSY_PIN);
    gpio_set_pin_output(EPD_RST_PIN);
    gpio_set_pin_output(EPD_DC_PIN);
    gpio_set_pin_output(EPD_CS_PIN);
    gpio_set_pin_output(EPD_SCK_PIN);
    gpio_set_pin_output(EPD_MOSI_PIN);

    gpio_write_pin_high(EPD_RST_PIN);
    gpio_write_pin_high(EPD_CS_PIN);
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

static void epd_send_command(uint8_t cmd)
{
    uint i;

    gpio_write_pin_low(EPD_DC_PIN);
    gpio_write_pin_low(EPD_CS_PIN);
    // Software SPI
    for (i = 0; i < 8; i++)
    {
        // Software SPI
        if ((cmd & 0x80) != 0)
        {
            gpio_write_pin_high(EPD_MOSI_PIN);
        }
        else
        {
            gpio_write_pin_low(EPD_MOSI_PIN);
        }
        cmd <<= 1;
        gpio_write_pin_high(EPD_SCK_PIN);
        gpio_write_pin_low(EPD_SCK_PIN);
    }
    gpio_write_pin_high(EPD_CS_PIN);
}

static void epd_send_data(uint8_t data)
{
    uint i;
    gpio_write_pin_high(EPD_DC_PIN);
    gpio_write_pin_low(EPD_CS_PIN);
    // Software SPI
    for (i = 0; i < 8; i++)
    {
        // Software SPI
        if ((data & 0x80) != 0)
        {
            gpio_write_pin_high(EPD_MOSI_PIN);
        }
        else
        {
            gpio_write_pin_low(EPD_MOSI_PIN);
        }
        data <<= 1;
        gpio_write_pin_high(EPD_SCK_PIN);
        gpio_write_pin_low(EPD_SCK_PIN);
    }
    gpio_write_pin_high(EPD_CS_PIN);
}


static void epd_send_data_block(const uint8_t* block, size_t len)
{
    uint i;
    uint j;
    uint8_t data;

    gpio_write_pin_high(EPD_DC_PIN);

    for (i = 0; i < len; i++)
    {
        gpio_write_pin_low(EPD_CS_PIN);
        // Software SPI
        data = block[i];
        for (j = 0; j < 8; j++)
        {
            // Software SPI
            if ((data & 0x80) != 0)
            {
                gpio_write_pin_high(EPD_MOSI_PIN);
            }
            else
            {
                gpio_write_pin_low(EPD_MOSI_PIN);
            }
            data <<= 1;
            gpio_write_pin_high(EPD_SCK_PIN);
            gpio_write_pin_low(EPD_SCK_PIN);
        }
        gpio_write_pin_high(EPD_CS_PIN);
    }
}

static void epd_send_data_repeatedly(const uint8_t data, size_t count)
{
    uint i;
    uint j;
    uint8_t temp;

    gpio_write_pin_high(EPD_DC_PIN);

    for (i = 0; i < count; i++)
    {
        gpio_write_pin_low(EPD_CS_PIN);
        // Software SPI
        temp = data;
        for (j = 0; j < 8; j++)
        {
            // Software SPI
            if ((temp & 0x80) != 0)
            {
                gpio_write_pin_high(EPD_MOSI_PIN);
            }
            else
            {
                gpio_write_pin_low(EPD_MOSI_PIN);
            }
            temp <<= 1;
            gpio_write_pin_high(EPD_SCK_PIN);
            gpio_write_pin_low(EPD_SCK_PIN);
        }
        gpio_write_pin_high(EPD_CS_PIN);
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
    epd_read_busy();
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

