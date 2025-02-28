#include <stdint.h>
#include "spi_master.h"


void send_data(const uint8_t* p, size_t len)
{
    uint i;

    for (i = 0; i < len; i++){
        spi_transmit(&p[i], 1);
    }
}

void foo(void)
{
    send_data((const uint8_t[]){ 0, 1, 2, 3, 4, 5}, 6);
}

