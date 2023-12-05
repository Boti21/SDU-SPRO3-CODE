#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
//#include "driver/adc.h"
#include "driver/gptimer.h"
#include "driver/timer.h"

#include "esp_log.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h" //New ADC libary


/* Misc macros */
#define CUSTOM_STACK_SIZE 2048
#define TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us

/* Pin macros */   // try to use macros for specific pins so they can be easily
#define PIN_INF 10

//GPIO 32 => ADC 1, CHANNEL 4
#define A4 ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use

/* Semaphores and Mutexes */
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;

/* Timer handle */
gptimer_handle_t timer = NULL;

/* ADC Handles */
adc_oneshot_unit_handle_t adc1_handle;

/* Prototypes */
//void init_adc(void);
void init_ultrasonic(void);
void init_adc_oneshot(void);


/*Global variables*/
uint8_t IR_CHANNELS[] = {0, 1, 3};
int inf_values[6];


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

    /*Init ADC*/
    init_adc_oneshot();

    /* Little boot up message ;) */
    char *main_name = pcTaskGetName(NULL);     // A way to get the name of the current task
    ESP_LOGI(main_name, "Program started..."); // The wayto print something to the terminal

    /* Task creation */
    //xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    //xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);

    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1);

    // Testing
    int adc_value = 0;

    while (1)
    {
        vTaskDelay(150 / portTICK_PERIOD_MS);
        /*
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
        */
        //Code to be removed
        adc_oneshot_read(adc1_handle, 0, &adc_value);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        ESP_LOGI(main_name, "ADC value: %d", adc_value);
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
}

/* Functions */

// Usage: after calling the init function
// calling the adc1_get_raw(A4) will return an int
// Conversion Vout = Dout* Vmax/Dmax
/*
void init_adc(void) {

    // Configuration of ADC
    adc1_config_channel_atten(A4, ADC_ATTEN_DB_11); // (channel we want to put for attenuation, which attenuation do we want)
    // Attenuation of db 11 is the biggest one, it allows the biggest range of reading
    adc1_config_width(ADC_WIDTH_BIT_12); // The resolution of all the adc1; 12 bits

    // Set pin GPO32 as an input
    gpio_num_t pinNumber = GPIO_NUM_32;
    gpio_set_direction(pinNumber, GPIO_MODE_INPUT);
}
*/

void init_adc_oneshot(void){
    //Creating oneshot handle
    /*
     * ADC configuration:
     * unit: ADC 1
     * ulp mode (Low power mode): Disable
     * Clock: Default
     */
    adc_oneshot_unit_init_cfg_t init_config1={
        .unit_id=ADC_UNIT_1,
        .ulp_mode=ADC_ULP_MODE_DISABLE
    };
    //Check if handle allocation was successful
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //Setting up ADC channels
    /*
    * ADC channel config:
    * atten: Input voltage of ADC attenuated extending the range of measurement by about 11 dB
    * bitwidth: default
    */
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    //Checking the channels
    //Testing code:
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, 0, &config));
    //Actual code:
    /*
    for (int i = 0; i < 8; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, i, &config));
    }
    */
    
}

void init_ultrasonic(void) {

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, 
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
    ESP_ERROR_CHECK(gptimer_enable(timer));

    /* Usage */
    /* The error checks are just for error checking... i know
    The timer is used by passing thesize_t handle of it to a function
    ESP_ERROR_CHECK(gptimer_start(timer)); // To start the timer
    ESP_ERROR_CHECK(gptimer_set_raw_count(timer, 0)); // Set the value of the timer
    ESP_ERROR_CHECK(gptimer_get_raw_count(timer, &timer_value)); // Get the value of the timer
    ESP_LOGI(main_name, "%llu", timer_value); // Printing, %llu stands for long long unsigned int OR uint64_t
    */

    // Comments so I won't forget
    /*
        One thread high priority
        does not get interrupted by anything
        pulses the sensor
        then waits until it gets a return blocking the core
        this will take a maxpvParametersimium of 38 ms of waiting so the whole
        task could be maybe around 40 ms
        this task can be fully periodic let's say at 10 Hz
        maybe there is a way to either make interrupts or events
        to signal the thread, but I have to look more into it
        40 ms is not long so it probably won't be a problem
    */
}

/*Check each adc value of infrared sensor*/
void infrared_adc_check(void){
    for (;;)
    {
        for (int i = 0; i < 5; i++)
        {
            adc_oneshot_read(adc1_handle, IR_CHANNELS[i], &inf_values[i]);
        }
    }
}
