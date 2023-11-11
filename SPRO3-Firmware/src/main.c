#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"

void app_main(void)
{
    char* task = pcTaskGetName(NULL);

    ESP_LOGI(task, "heeeeeeeeeeeeeeeeeey");

    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    

    while(1) {
        ESP_LOGI(task, "loop iteration\n");
        gpio_set_level(2, 1);
        vTaskDelay(50);
        gpio_set_level(2, 0);
        vTaskDelay(50);
    }
}