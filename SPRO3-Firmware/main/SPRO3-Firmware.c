#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_log.h"
#include "esp_adc_cal.h"

// Semaphores and Mutexes
SemaphoreHandle_t screen_mutex;

/* Pin macros */
#define ADC_INPUT 

void test_task(void* pvParameters) {
    char* test_task_name = pcTaskGetName(NULL);

    for(;;) {
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(test_task_name, "task");
        xSemaphoreGive(screen_mutex);

        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}

void test_task2(void* pvParameters) {
    char* test_task_name2 = pcTaskGetName(NULL);

    for(;;) {
        //xSemaphoreTake(screen_mutex, portMAX_DELAY);
        //ESP_LOGI(test_task_name2, "task2");
        //xSemaphoreGive(screen_mutex);

        gpio_set_level(2, 1);
        vTaskDelay(150);
        gpio_set_level(2, 0);
        vTaskDelay(150);
    }
}

void app_main(void)
{
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);

    char* main_name = pcTaskGetName(NULL);
    ESP_LOGI(main_name, "this is a task");

    screen_mutex = xSemaphoreCreateMutex();

    TaskHandle_t test_handle = NULL;
    xTaskCreate(test_task, "test_task", 1000, NULL, 2, &test_handle);

    TaskHandle_t test_handle2 = NULL;
    xTaskCreate(test_task2, "test_task2", 1000, NULL, 2, &test_handle2);

    while(1) {
        vTaskDelay(150 / portTICK_PERIOD_MS);
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
    }

}

