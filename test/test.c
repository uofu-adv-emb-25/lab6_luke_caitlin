#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include "unity_config.h"
#include <unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void supervisor(void *params)
{
    while (1) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN();
        UNITY_END();
    }
}

int main(void)
{
    stdio_init_all();

    xTaskCreate(supervisor, "supervisor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+4, NULL);
    vTaskStartScheduler();
}
