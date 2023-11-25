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

/* Misc macros */
#define CUSTOM_STACK_SIZE 2048

/* Pin macros */   // try to use macros for specific pins so they can be easily reassigned

//GPIO 32 => ADC 1, CHANNEL 4
#define A4 ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPO we want to use

/* Prototypes */
void init_adc(void);


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

void app_main(void)
{
    /* Initializing mutexes and semaphores */
    screen_mutex = xSemaphoreCreateMutex();

    /* Little boot up message ;) */
    char *main_name = pcTaskGetName(NULL);     // A way to get the name of the current task
    ESP_LOGI(main_name, "Program started..."); // The wayto print something to the terminal

    /* Task handles and task creation */
    TaskHandle_t test_handle = NULL;
    xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);

    TaskHandle_t test_handle2 = NULL;
    xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);

    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);

    while (1)
    {
        vTaskDelay(150 / portTICK_PERIOD_MS);
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
    }
}

/* Functions */

// Usage: after calling the init function
// calling the adc1_get_raw(A4) will return an int
// Conversion Vout = Dout* Vmax/Dmax
void init_adc(void){

    // Configuration of ADC
    adc1_config_channel_atten(A4, ADC_ATTEN_DB_11); // (channel we want to put for attenuation, which attenuation do we want)
    // Attenuation of db 11 is the biggest one, it allows the biggest range of reading
    adc1_config_width(ADC_WIDTH_BIT_12); // The resolution of all the adc1; 12 bits

    // Set pin GPO32 as an input
    gpio_num_t pinNumber = GPIO_NUM_32;
    gpio_set_direction(pinNumber, GPIO_MODE_INPUT);
}
