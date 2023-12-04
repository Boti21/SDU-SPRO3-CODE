#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/gptimer.h"
#include "driver/timer.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "esp_adc_cal.h"
#include "esp_err.h"


/* Misc macros */
#define CUSTOM_STACK_SIZE 2048
#define TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us

/* Pin macros */   // try to use macros for specific pins so they can be easily reassigned
#define Mast_motor  0
#define Left_motor  1
#define Right_motor 2
#define Mast_motor_GPIO 5
#define Left_motor_GPIO  18
#define Right_motor_GPIO 19
//GPIO 32 => ADC 1, CHANNEL 4
#define A4 ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use

/* Semaphores and Mutexes */
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;
TaskHandle_t monitor_handle = NULL;

/* Timer handle */
gptimer_handle_t timer = NULL;

/* Prototypes */
void init_adc(void);
void init_ultrasonic(void);
void init_pwm(int, int);

void pwm_start();
void pwm_stop();


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
    ESP_LOGI(main_name, "Program started..."); // The wayto print something to the terminal

    /* Task creation */
    //xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    //xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);
    //xTaskCreate(monitor_task, "monitor_task", CUSTOM_STACK_SIZE, NULL, 2, &monitor_handle);

    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1);

    // Testing
    //initalise pwm
    init_pwm(Mast_motor,Mast_motor_GPIO);
    init_pwm(Left_motor,Left_motor_GPIO);
    init_pwm(Right_motor,Right_motor_GPIO);

    while (1)
    {
        ESP_LOGI(main_name, "Main loop...");
        vTaskDelay(150 / portTICK_PERIOD_MS);
        //speed control 0-255
        pwm_start(Mast_motor,250);
        pwm_start(Left_motor,250);
        pwm_start(Right_motor,250);

        vTaskDelay(5000 / portTICK_PERIOD_MS);

        pwm_stop(Mast_motor);
        pwm_stop(Left_motor);
        pwm_stop(Right_motor);
        vTaskDelay(500 / portTICK_PERIOD_MS);
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
void init_adc(void)
{

    // Configuration of ADC
    adc1_config_channel_atten(A4, ADC_ATTEN_DB_11); // (channel we want to put for attenuation, which attenuation do we want)
    // Attenuation of db 11 is the biggest one, it allows the biggest range of reading
    adc1_config_width(ADC_WIDTH_BIT_12); // The resolution of all the adc1; 12 bits

    // Set pin GPO32 as an input
    gpio_num_t pinNumber = GPIO_NUM_32;
    gpio_set_direction(pinNumber, GPIO_MODE_INPUT);
}

void init_ultrasonic(void)
{

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
    ESP_ERROR_CHECK(gptimer_enable(timer));

    /* Usage */
    /* The error checks are just for error checking... i know
    The timer is used by passing the handle of it to a function
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
        this will take a maximium of 38 ms of waiting so the whole
        task could be maybe around 40 ms
        this task can be fully periodic let's say at 10 Hz
        maybe there is a way to either make interrupts or events
        to signal the thread, but I have to look more into it
        40 ms is not long so it probably won't be a problem
    */
}

void init_pwm(int motor, int GPIO)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = 0, // Low speed mode
        .duty_resolution  = 8,// resolution in bit, 
        .timer_num        = 0, // timer x
        .freq_hz          = 1000,// Hz
        .clk_cfg          = 0 // configures clock automatically 
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = 0, // Low speed mode 
        .channel        = motor,
        .timer_sel      = 0,
        .intr_type      = 0, // disable interrupts
        .gpio_num       = GPIO,
        .duty           = 128, //Set duty to 50% in 8 bit :  (2**8)*50% = 128
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    ledc_timer_pause(0,motor);
}

void pwm_start(int motor, int duty){
    ledc_set_duty(0,motor,duty);   
    ledc_timer_resume(0,motor);
}
void pwm_stop(int motor){
   ledc_timer_pause(0,motor);
}