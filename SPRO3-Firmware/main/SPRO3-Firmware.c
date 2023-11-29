#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/timer.h"

#include "esp_log.h"
#include "esp_adc_cal.h"


/* Misc macros */
#define CUSTOM_STACK_SIZE 2048
#define TIMER_GROUP 0
#define TIMER 0
#define TIMER_PRESCALER (uint32_t)80 //80 // 80 Mhz input signal --> microseconds counted

/* Pin macros */   // try to use macros for specific pins so they can be easily reassigned

//GPIO 32 => ADC 1, CHANNEL 4
#define A4 ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use

// Semaphores and Mutexes
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;

/* Prototypes */
void init_adc(void);
void init_ultrasonic(void);


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

    /* Task creation */
    //xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    //xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);

    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);

    // Testing
    long unsigned int timer_val = 0;
    init_ultrasonic();

    while (1)
    {
        timer_start((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER);
        vTaskDelay(150 / portTICK_PERIOD_MS);
        timer_get_counter_value((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, &timer_val);
        ESP_LOGI(main_name, "%lu", timer_val);
        //timer_pause((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER);
        //timer_set_counter_value((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, 0);
        /*
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
        */
    }
}

/* Functions */

// Usage: after calling the init function
// calling the adc1_get_raw(A4) will return an int
// Conversion Vout = Dout* Vmax/Dmax
void init_adc(void) {

    // Configuration of ADC
    adc1_config_channel_atten(A4, ADC_ATTEN_DB_11); // (channel we want to put for attenuation, which attenuation do we want)
    // Attenuation of db 11 is the biggest one, it allows the biggest range of reading
    adc1_config_width(ADC_WIDTH_BIT_12); // The resolution of all the adc1; 12 bits

    // Set pin GPO32 as an input
    gpio_num_t pinNumber = GPIO_NUM_32;
    gpio_set_direction(pinNumber, GPIO_MODE_INPUT);
}

void init_ultrasonic(void) {

    timer_config_t test;
    test.alarm_en = TIMER_ALARM_DIS;
    test.counter_en = TIMER_PAUSE;
    test.intr_type = TIMER_INTR_NONE;
    test.counter_dir = TIMER_COUNT_UP;
    test.auto_reload = TIMER_AUTORELOAD_DIS;
    //test.clk_src = 0;
    test.divider = TIMER_PRESCALER;
    //timer_src_clk_t clk_src;        /*!< Selects source clock. */
    timer_init((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, &test);

    timer_set_divider((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, TIMER_PRESCALER);
    timer_set_counter_mode((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, TIMER_COUNT_UP);
    timer_set_auto_reload((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, TIMER_AUTORELOAD_DIS);
    timer_set_counter_value((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, 0);
    timer_init((timer_group_t)TIMER_GROUP, (timer_idx_t)TIMER, &test);

    // Comments so I won't forget
    /*
        One thread high priority
        does not get interrupted by anything
        pulses the sensor
        then waits until it gets a return blocking the core
        this will take a maximium of 38 ms of waiting so the whole
        task could be maybe around 40 ms
        this task can be fully periodic let's say at 10 Hz
        maybe there is a way to either make interrupts or events
        to signal the thread, but I have to look more into it
        40 ms is not long so it probably won't be a problem
    */
}
