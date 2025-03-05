// Waveshare 12956
// /296x128, 2.9inch E-Ink display module driver
#include QMK_KEYBOARD_H
#include <hardware/structs/systick.h>
#include "spi_master.h"
#include "epd_ws1in54_V2.h"


#define EPD_BUSY_PIN    GP10
#define EPD_MOSI_PIN    SPI_MOSI_PIN
#define EPD_SCK_PIN     SPI_SCK_PIN
#define EPD_CS_PIN      GP20
#define EPD_DC_PIN      GP26
#define EPD_RST_PIN     GP27


const uint32_t HW_RESET_MS_HIGH_PRE = 10;
const uint32_t HW_RESET_MS_LOW = 2;
const uint32_t HW_RESET_MS_HIGH_POST = 10;
const uint32_t DEEP_SLEEP_MS_DELAY = 100;
const uint32_t ENTER_DEEP_SLEEP_AFTER_MS = 3000;

inline uint low_byte(uint value)
{
    return value & 0xff;
}

inline uint high_byte(uint value)
{
    return (value >> 8) & 0xff;
}

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
    bool first = true;
    while (gpio_read_pin(EPD_BUSY_PIN)){
        if (first){
            dprintf("send command %02x in spite of BUSY\n", cmd);
            first = false;
        }
        wait_ms(1);
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

///////////////////////////////////////////////////////////////
// Control Sequencer Engine

typedef enum cseq_type {
    CSEQ_TYPE_END = 0,
    CSEQ_TYPE_FUNC_BV,
    CSEQ_TYPE_FUNC_BCVP
} cseq_type_t;

typedef bool (*cseq_func_bv_t)(void);
typedef bool (*cseq_func_bcvp_t)(const void*);

typedef union cseq_elem   cseq_elem_t;

typedef struct cseq_elem_header {
    cseq_type_t type;
} cseq_elem_header_t;

typedef struct cseq_elem_func_bv {
    cseq_type_t type;
    cseq_func_bv_t func;
} cseq_elem_func_bv_t;

typedef struct cseq_elem_func_bcvp {
    cseq_type_t type;
    cseq_func_bcvp_t func;
    void* param;
} cseq_elem_func_bcvp_t;

union cseq_elem {
    cseq_elem_header_t  header;
    cseq_elem_func_bv_t func_bv;
    cseq_elem_func_bcvp_t func_bcvp;
};

#define BEGIN_SEQ(NAME) const cseq_elem_t NAME[] = {
#define END_SEQ {.header={.type=CSEQ_TYPE_END}}};

#define SEQELEM_FUNC_BV(FUNC)  {.func_bv={.type=CSEQ_TYPE_FUNC_BV, .func=(FUNC)}}
#define SEQELEM_FUNC_BCVP(FUNC, PARAM)  {.func_bcvp={.type=CSEQ_TYPE_FUNC_BCVP, .func=(const void*)(FUNC), .param=(PARAM)}}
#define SEQELEM_DPRINT(MSG) {.func_bcvp={.type=CSEQ_TYPE_FUNC_BCVP, .func=(const void*)seqelem_dprint, .param=(MSG)}}

static const cseq_elem_t*   seq_next;
static const cseq_elem_t*   pelem_cur;
static const uint8_t* image_next;
static const uint8_t* image;
static uint32_t tim_seq_start;


static void cseq_engine_task(void)
{
    if (pelem_cur == NULL){
        if (seq_next == NULL){
            return;
        }
        pelem_cur = seq_next;
        seq_next = NULL;
        image = image_next;
        image_next = NULL;
        tim_seq_start = timer_read32();
    }
    switch (pelem_cur->header.type){
        case CSEQ_TYPE_END:
            dprintf("seq running time: %lu\n", timer_elapsed32(tim_seq_start));
            pelem_cur = NULL;
            return;
        case CSEQ_TYPE_FUNC_BV:
            {
                const cseq_elem_func_bv_t* pefunc_bv = (const cseq_elem_func_bv_t*)pelem_cur;
                if ((pefunc_bv->func)() == false){
                    return;
                }
                break;
            }
        case CSEQ_TYPE_FUNC_BCVP:
            {
                const cseq_elem_func_bcvp_t* pefunc_bcvp = (const cseq_elem_func_bcvp_t*)pelem_cur;
                if ((pefunc_bcvp->func)(pefunc_bcvp->param) == false){
                    return;
                }
                break;
            }
    }
    pelem_cur++;
}

static void cseq_start(const cseq_elem_t* seq, const uint8_t* image)
{
    seq_next = seq;
    image_next = image;
}

// Control Sequencer Engine
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Elements of Sequence

typedef struct {
    uint32_t    tim_start;
    uint32_t    ms_delay;
} delay_ctx_t;

static delay_ctx_t delay_ctx;

void delay_async(uint32_t ms_delay)
{
    delay_ctx.ms_delay = ms_delay;
    delay_ctx.tim_start = timer_read32();
}

bool delay_wait(void)
{
    if (timer_elapsed32(delay_ctx.tim_start) < delay_ctx.ms_delay){
        return false;
    }
    return true;
}


typedef enum {
    HW_RESET_PHASE_OUTSET = 0,
    HW_RESET_PHASE_HIGH_PRE,
    HW_RESET_PHASE_LOW,
    HW_RESET_PHASE_HIGH_POST,
    HW_RESET_PHASE_COMPL
} hw_reset_phase_t;

static hw_reset_phase_t hw_reset_phase = HW_RESET_PHASE_OUTSET;
static uint32_t tim_start_turn_on_display;

static bool seqelem_dprint(const char* msg)
{
    dprint(msg);
    return true;
}

static bool seqelem_delay_init_async(void)
{
    delay_async(3000);
    return true;
}

static bool seqelem_delay_init_wait(void)
{
    return delay_wait();
}

static bool seqelem_hw_reset_async(void)
{
    // 初回なら HW RESET する。
    // 初回なら hw_reset_phase == HW_RESET_PHASE_OUTSET
    // BUSYならDeep Sleep中と見做して HW RESET する。
    if (hw_reset_phase != HW_RESET_PHASE_OUTSET && gpio_read_pin(EPD_BUSY_PIN) == 0){
        dprint("hw_reset skip\n");
        return true;
    }
    dprint("hw_reset\n");
    hw_reset_phase = HW_RESET_PHASE_HIGH_PRE;
    gpio_write_pin_high(EPD_RST_PIN);
    delay_async(HW_RESET_MS_HIGH_PRE);
    return true;
}

static bool seqelem_hw_reset_wait(void)
{
    switch (hw_reset_phase){
        case HW_RESET_PHASE_OUTSET:
            dprint("hw reset sequence error.\n");
            break;
        case HW_RESET_PHASE_HIGH_PRE:
            if (delay_wait() == false){
                return false;
            }
            hw_reset_phase = HW_RESET_PHASE_LOW;
            gpio_write_pin_low(EPD_RST_PIN);
            delay_async(HW_RESET_MS_LOW);
            return false;
        case HW_RESET_PHASE_LOW:
            if (delay_wait() == false){
                return false;
            }
            hw_reset_phase = HW_RESET_PHASE_HIGH_POST;
            gpio_write_pin_high(EPD_RST_PIN);
            delay_async(HW_RESET_MS_HIGH_POST);
            return false;
        case HW_RESET_PHASE_HIGH_POST:
            if (delay_wait() == false){
                return false;
            }
            hw_reset_phase = HW_RESET_PHASE_COMPL;
            return true;
        case HW_RESET_PHASE_COMPL:
            break;
    }
    return true;
}

static bool seqelem_sw_reset_async(void)
{
    // soft reset
    dprint("sw reset\n");
    epd_send_command(0x12);
    return true;
}

static bool seqelem_sw_reset_wait(void)
{
    if (gpio_read_pin(EPD_BUSY_PIN)){
        return false;
    }
    return true;
}

static bool seqelem_init_async(void)
{
    // Driver output control
    epd_send_command(0x01);
    //epd_send_data(low_byte(EPD_HEIGHT - 1));
    //epd_send_data(high_byte(EPD_HEIGHT - 1));
    epd_send_data(0xc7);
    epd_send_data(0x00);
    epd_send_data(0x00);

    // Data entry mode
    epd_send_command(0x11);
    epd_send_data(0x03);

    // Select border waveform for VBD
    epd_send_command(0x3c);
    epd_send_data(0x05);

    // Temperature Sensor Selection
    epd_send_command(0x18);
    epd_send_data(0x80); // Internal temperature sensor
    return true;
}

static bool seqelem_init_wait(void)
{
    return true;
}

static bool seqelem_init_partial_async(void)
{
    return true;
}

static bool seqelem_init_partial_wait(void)
{
    return true;
}

static bool seqelem_send_image(void)
{
    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x24);
    epd_send_data_block(image, COUNT_BYTES_IN_IMAGE);
    return true;
}

static bool seqelem_send_image_back(void)
{
    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x26);
    epd_send_data_block(image, COUNT_BYTES_IN_IMAGE);
    return true;
}

static bool seqelem_clear_back(void)
{
    epd_set_windows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
    epd_set_cursor(0, 0);

    epd_send_command(0x26);
    epd_send_data_repeatedly(0xff, COUNT_BYTES_IN_IMAGE);
    return true;
}

static bool seqelem_turn_on_display_async(void)
{
    tim_start_turn_on_display = timer_read32();
    epd_send_command(0x22);
    epd_send_data(0xf7);
    epd_send_command(0x20);
    return true;
}

static bool seqelem_turn_on_display_wait(void)
{
    if (gpio_read_pin(EPD_BUSY_PIN)){
        return false;
    }
    dprintf("turn on display: %lu[ms]\n", timer_elapsed32(tim_start_turn_on_display));
    return true;
}

static bool seqelem_turn_on_display_partial_async(void)
{
    tim_start_turn_on_display = timer_read32();
    epd_send_command(0x22);
    epd_send_data(0xff);
    epd_send_command(0x20);
    return true;
}

static bool seqelem_turn_on_display_partial_wait(void)
{
    if (gpio_read_pin(EPD_BUSY_PIN)){
        return false;
    }
    dprintf("turn on display partial: %lu[ms]\n", timer_elapsed32(tim_start_turn_on_display));
    return true;
}

static bool seqelem_deep_sleep_async(void)
{
    dprint("deep sleep\n");
    epd_send_command(0x10);
    epd_send_data(0x01);
    hw_reset_phase = HW_RESET_PHASE_OUTSET;
    delay_async(DEEP_SLEEP_MS_DELAY);
    return true;
}

static bool seqelem_deep_sleep_wait(void)
{
    if (delay_wait() == false){
        return false;
    }
    dprintf("after deep sleep. BUSY:%lu\n", gpio_read_pin(EPD_BUSY_PIN));
    return true;
}

// Elements of Sequence
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Sequences

BEGIN_SEQ(seq_init)
    SEQELEM_DPRINT("seq_init start\n"),
    SEQELEM_FUNC_BV(seqelem_delay_init_async),
    SEQELEM_FUNC_BV(seqelem_delay_init_wait),
    SEQELEM_FUNC_BV(seqelem_hw_reset_async),
    SEQELEM_FUNC_BV(seqelem_hw_reset_wait),
    SEQELEM_FUNC_BV(seqelem_sw_reset_async),
    SEQELEM_FUNC_BV(seqelem_sw_reset_wait),
    SEQELEM_FUNC_BV(seqelem_init_async),
    SEQELEM_FUNC_BV(seqelem_init_wait),
    SEQELEM_FUNC_BV(seqelem_send_image),
    SEQELEM_FUNC_BV(seqelem_clear_back),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_async),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_wait),
    SEQELEM_FUNC_BV(seqelem_send_image_back),
    SEQELEM_DPRINT("seq_init end\n"),
END_SEQ

BEGIN_SEQ(seq_show_image)
    SEQELEM_DPRINT("seq_show_image start\n"),
    SEQELEM_FUNC_BV(seqelem_hw_reset_async),
    SEQELEM_FUNC_BV(seqelem_hw_reset_wait),
    SEQELEM_FUNC_BV(seqelem_send_image),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_async),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_wait),
    SEQELEM_FUNC_BV(seqelem_send_image_back),
    SEQELEM_DPRINT("seq_show_image end\n"),
END_SEQ

BEGIN_SEQ(seq_show_image_partial)
    SEQELEM_DPRINT("seq_show_image_partial start\n"),
    SEQELEM_FUNC_BV(seqelem_hw_reset_async),
    SEQELEM_FUNC_BV(seqelem_hw_reset_wait),
    SEQELEM_FUNC_BV(seqelem_send_image),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_partial_async),
    SEQELEM_FUNC_BV(seqelem_turn_on_display_partial_wait),
    SEQELEM_FUNC_BV(seqelem_send_image_back),
    SEQELEM_DPRINT("seq_show_image_partial end\n"),
END_SEQ

BEGIN_SEQ(seq_deep_sleep)
    SEQELEM_DPRINT("seq_deep_sleep start\n"),
    SEQELEM_FUNC_BV(seqelem_deep_sleep_async),
    SEQELEM_FUNC_BV(seqelem_deep_sleep_wait),
    SEQELEM_DPRINT("seq_deep_sleep end\n"),
END_SEQ

// Sequences
////////////////////////////////////////////////////

typedef enum {
    IDLE_TIMER_PHASE_FREE = 0,
    IDLE_TIMER_PHARE_START,
    IDLE_TIMER_PHARE_EXPIRED
} idle_timer_phase_t;

static idle_timer_phase_t idle_timer_phase = IDLE_TIMER_PHASE_FREE;
static uint32_t tim_idle_start;


void epd_display_image(const uint8_t* image)
{
    idle_timer_phase = IDLE_TIMER_PHASE_FREE;
    cseq_start(seq_show_image, image);
}

void epd_display_image_partial(const uint8_t* image)
{
    idle_timer_phase = IDLE_TIMER_PHASE_FREE;
    cseq_start(seq_show_image_partial, image);
}

void epd_task_init(const uint8_t* image)
{
    idle_timer_phase = IDLE_TIMER_PHASE_FREE;
    epd_gpio_config();
    cseq_start(seq_init, image);
}

void epd_task(void)
{
    cseq_engine_task();

    // After ENTER_DEEP_SLEEP_AFTER_MS of not using the device,
    // put the device to deep sleep.
    if (pelem_cur == NULL && gpio_read_pin(EPD_BUSY_PIN) == 0){
        switch (idle_timer_phase){
            case IDLE_TIMER_PHASE_FREE:
                dprint("idle timer start\n");
                idle_timer_phase = IDLE_TIMER_PHARE_START;
                tim_idle_start = timer_read32();
                break;
            case IDLE_TIMER_PHARE_START:
                if (timer_elapsed32(tim_idle_start) < ENTER_DEEP_SLEEP_AFTER_MS){
                    break;
                }
                dprint("idle timer expired\n");
                idle_timer_phase = IDLE_TIMER_PHARE_EXPIRED;
                cseq_start(seq_deep_sleep, NULL);
                break;
            case IDLE_TIMER_PHARE_EXPIRED:
                break;
        }
    }
}

// 参照していないstaticな関数がエラーになるので、ここで参照しておく
void dummy(void)
{
//    cseq_engine_task()
    seqelem_init_partial_async();
    seqelem_init_partial_wait();
    seqelem_deep_sleep_async();
    seqelem_deep_sleep_wait();
}
