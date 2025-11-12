#include <pico/stdlib.h>

void timer_configure(void)
{
}

uint32_t timer_count(void)
{
    return time_us_32();
}
