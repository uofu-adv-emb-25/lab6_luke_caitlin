#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "unity_config.h"
#include <unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void take_lock(void *params)
{
    SemaphoreHandle_t *semaphore = params;
    xSemaphoreTake(*semaphore, portMAX_DELAY);
    vTaskSuspend(NULL);
    xSemaphoreGive(*semaphore);
    for (int i = 0; ; i++);
}

void busy_busy(void *params)
{
    for (int i = 0; ; i++);
}

void busy_yield(void *params)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}

void test_priority_inversion(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;
    TaskHandle_t task_3;

    SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphore);
    xTaskCreate(take_lock, "task_1", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+1, &task_1);
    vTaskDelay(100);
    xTaskCreate(busy_busy, "task_2", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+2, &task_2);
    xTaskCreate(take_lock, "task_3", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+3, &task_3);

    vTaskDelay(200);

    TEST_ASSERT(eTaskGetState(task_1) == eSuspended);
    TEST_ASSERT(eTaskGetState(task_2) == eReady);
    TEST_ASSERT(eTaskGetState(task_3) == eBlocked);

    vTaskDelete(task_1);
    vTaskDelete(task_2);
    vTaskDelete(task_3);
}

void test_priority_inheritance(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;
    TaskHandle_t task_3;

    SemaphoreHandle_t semaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(semaphore);
    xTaskCreate(take_lock, "task_1", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+1, &task_1);
    vTaskDelay(100);
    xTaskCreate(busy_busy, "task_2", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+2, &task_2);
    xTaskCreate(take_lock, "task_3", configMINIMAL_STACK_SIZE, &semaphore, tskIDLE_PRIORITY+3, &task_3);

    vTaskDelay(200);

    TEST_ASSERT(eTaskGetState(task_1) == eSuspended);
    TEST_ASSERT(eTaskGetState(task_2) == eReady);
    TEST_ASSERT(eTaskGetState(task_3) == eBlocked);

    vTaskResume(task_1);
    vTaskDelay(200);

    TEST_ASSERT(eTaskGetState(task_1) == eReady);
    TEST_ASSERT(eTaskGetState(task_2) == eReady);
    TEST_ASSERT(eTaskGetState(task_3) == eSuspended);

    vTaskDelete(task_1);
    vTaskDelete(task_2);
    vTaskDelete(task_3);
}

void test_busy_busy(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;

    xTaskCreate(busy_busy, "task_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_1);
    xTaskCreate(busy_busy, "task_2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_2);

    vTaskDelay(200);

    configRUN_TIME_COUNTER_TYPE time_1 = ulTaskGetRunTimeCounter(task_1);
    configRUN_TIME_COUNTER_TYPE time_2 = ulTaskGetRunTimeCounter(task_2);
    
    printf("Task1 runtime: %lu, Task2 runtime: %lu, ratio: %.2f\n",
       time_1, time_2, (float)time_1 / (float)(time_2 + 1));

    vTaskDelete(task_1);
    vTaskDelete(task_2);

}

void test_busy_yield(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;

    xTaskCreate(busy_yield, "task_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_1);
    xTaskCreate(busy_yield, "task_2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_2);

    vTaskDelay(200);

    configRUN_TIME_COUNTER_TYPE time_1 = ulTaskGetRunTimeCounter(task_1);
    configRUN_TIME_COUNTER_TYPE time_2 = ulTaskGetRunTimeCounter(task_2);
    
    printf("Task1 runtime: %lu, Task2 runtime: %lu, ratio: %.2f\n",
       time_1, time_2, (float)time_1 / (float)(time_2 + 1));

    vTaskDelete(task_1);
    vTaskDelete(task_2);
}

void test_busy_both(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;

    xTaskCreate(busy_busy, "task_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_1);
    xTaskCreate(busy_yield, "task_2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_2);

    vTaskDelay(200);

    configRUN_TIME_COUNTER_TYPE time_1 = ulTaskGetRunTimeCounter(task_1);
    configRUN_TIME_COUNTER_TYPE time_2 = ulTaskGetRunTimeCounter(task_2);
    
    printf("Task1 runtime: %lu, Task2 runtime: %lu, ratio: %.2f\n",
       time_1, time_2, (float)time_1 / (float)(time_2 + 1));

    vTaskDelete(task_1);
    vTaskDelete(task_2);
}

void test_busy_priority(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;

    xTaskCreate(busy_busy, "task_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &task_1);
    xTaskCreate(busy_busy, "task_2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_2);

    vTaskDelay(200);

    configRUN_TIME_COUNTER_TYPE time_1 = ulTaskGetRunTimeCounter(task_1);
    configRUN_TIME_COUNTER_TYPE time_2 = ulTaskGetRunTimeCounter(task_2);
    
    printf("Task1 runtime: %lu, Task2 runtime: %lu, ratio: %.2f\n",
       time_1, time_2, (float)time_1 / (float)(time_2 + 1));

    vTaskDelete(task_1);
    vTaskDelete(task_2);
}

void test_yield_priority(void)
{
    TaskHandle_t task_1;
    TaskHandle_t task_2;

    xTaskCreate(busy_yield, "task_1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &task_1);
    xTaskCreate(busy_yield, "task_2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &task_2);

    vTaskDelay(200);

    configRUN_TIME_COUNTER_TYPE time_1 = ulTaskGetRunTimeCounter(task_1);
    configRUN_TIME_COUNTER_TYPE time_2 = ulTaskGetRunTimeCounter(task_2);
    
    printf("Task1 runtime: %lu, Task2 runtime: %lu, ratio: %.2f\n",
       time_1, time_2, (float)time_1 / (float)(time_2 + 1));

    vTaskDelete(task_1);
    vTaskDelete(task_2);
}


void supervisor(void *params)
{
    while(1){
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_priority_inversion);
        RUN_TEST(test_priority_inheritance);
        RUN_TEST(test_busy_busy);
        RUN_TEST(test_busy_yield);
        RUN_TEST(test_busy_both);
        RUN_TEST(test_busy_priority);
        RUN_TEST(test_yield_priority);
        UNITY_END();
    }
}

int main(void)
{
    stdio_init_all();

    xTaskCreate(supervisor, "supervisor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+4, NULL);
    vTaskStartScheduler();
}
