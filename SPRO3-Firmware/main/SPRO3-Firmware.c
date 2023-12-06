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
#include "FL_ultrasonic.h"
#include "FL_PWM.h"
#include "FL_ADCandIR.h"

/* Misc macros */
#define CUSTOM_STACK_SIZE 2048

#define ADC1_CHANNELS_NUM 8 // Number of ADC channels on ADC unit 1
#define ADC2_CHANNELS_NUM 2 // (for now) Number of ADC channels on ADC unit 2

/* Pin macros */   // try to use macros for specific pins so they can be easily reassigned






#define LOAD_CELL_GPIO ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use

/* Semaphores and Mutexes */
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;
TaskHandle_t monitor_handle = NULL;





void monitor_task(void* pvParameters) {
    for(;;) {
        // Functions which check for battery voltage and collision detection (and communication)
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

void app_main(void)
{
    /* Initializing mutexes and semaphores */
    screen_mutex = xSemaphoreCreateMutex();


    /* Little boot up message ;) */
    char *main_name = pcTaskGetName(NULL);     // A way to get the name of the current task
    ESP_LOGI(main_name, "Program started..."); // The way to print something to the terminal

    /* Task creation */
    //xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    //xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);
    //xTaskCreate(monitor_task, "monitor_task", CUSTOM_STACK_SIZE, NULL, 2, &monitor_handle);

    /* Init functions */
    init_adc();
    init_pwm(M_MOTOR, M_MOTOR_GPIO);
    init_pwm(L_MOTOR, L_MOTOR_GPIO);
    init_pwm(R_MOTOR, R_MOTOR_GPIO);
    init_ultrasonic();

    // Testing
    int adc_value = 0;
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1);


    while (1)
    {
        ESP_LOGI(main_name, "Main loop...");
        vTaskDelay(150 / portTICK_PERIOD_MS);
        /*
        pwm_start(M_MOTOR, 250);
        pwm_start(L_MOTOR, 250);
        pwm_start(R_MOTOR, 250);
        */
        
        // Measure distance to Object
        double dis_to_obj = distance_ultrasonic();
        ESP_LOGI("DisFunc", "DisFunc %f",dis_to_obj);

        // Read IR-SENSOR in the front

        infrared_adc_check_front();

        for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {

            ESP_LOGI("IR_RESULTS_FRONT", "Val %d: %d", i, inf_values_front[i]);
        
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        /*
        pwm_stop(M_MOTOR);
        pwm_stop(L_MOTOR);
        pwm_stop(R_MOTOR);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        */
        /*
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
        */


        //Code to be removed
        //adc_oneshot_read(adc1_handle, IR_CHANNELS_FRONT[0], &adc_value);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        ESP_LOGI(main_name, "ADC value: %d", adc_value);
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
}

/* Functions */





