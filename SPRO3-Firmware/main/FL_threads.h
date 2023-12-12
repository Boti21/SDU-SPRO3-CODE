#ifndef FL_THREADS_H
#define FL_THREADS_H
#endif

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "esp_err.h"

// Custom includes
#ifndef FL_ADC_IR_H // Fucking black magic
#include "FL_ultrasonic.h"
#include "FL_drive.h"
#include "FL_ADC_IR.h"
#endif

#include "FL_endstop.h"

/* Misc macros */
#define CUSTOM_STACK_SIZE 2048

/* Semaphores and Mutexes */
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;
TaskHandle_t monitor_handle = NULL;


void monitor_task(void* pvParameters) {
    for(;;) {
        // Functions which check for battery voltage and collision detection (and communication)
        if(check_endstop_up() == 1) {
            // Endstop reached
        } else if(check_endstop_down() == 1) {
            // Endstop reached
        }
    }
}


void test_task(void *pvParameters)
{
    char *test_task_name = pcTaskGetName(NULL);

    for (;;)
    {
        xSemaphoreTake(screen_mutex, portMAX_DELAY); // Critical region started
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(test_task_name, "task");
        xSemaphoreGive(screen_mutex); // Critical region ended

        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}

void test_task2(void *pvParameters)
{
    char *test_task_name2 = pcTaskGetName(NULL);

    for (;;)
    {
        xSemaphoreTake(screen_mutex, portMAX_DELAY); // Critical region started
        ESP_LOGI(test_task_name2, "task2");
        xSemaphoreGive(screen_mutex); // Critical region ended

        gpio_set_level(2, 1);
        vTaskDelay(150);
        gpio_set_level(2, 0);
        vTaskDelay(150);
    }
}
