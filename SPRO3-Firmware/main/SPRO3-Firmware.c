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


/* Misc macros */
#define CUSTOM_STACK_SIZE 2048
#define TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us
#define ADC1_CHANNELS_NUM 8 // Number of ADC channels on ADC unit 1
#define ADC2_CHANNELS_NUM 0 // Number of ADC channels on ADC unit 2

/* Pin macros */   // try to use macros for specific pins so they can be easily reassigned
#define M_MOTOR 0 // Mast motor
#define L_MOTOR 1 // Left motor (facing the same way as the fork)
#define R_MOTOR 2 // Right motor (facing the same way as the fork)
#define M_MOTOR_GPIO 5 // TBD
#define L_MOTOR_GPIO 18 // TBD
#define R_MOTOR_GPIO 19 // TBD

// Most of these will probably change and these placeholders
#define IR_FRONT_0_GPIO 0 // TBD
#define IR_FRONT_1_GPIO 0 // TBD
#define IR_FRONT_3_GPIO 0 // TBD
#define IR_FRONT_4_GPIO 0 // TBD
#define IR_FRONT_5_GPIO 0 // TBD
#define IR_FRONT_6_GPIO 0 // TBD
#define IR_FRONT_7_GPIO 0 // TBD
#define IR_FRONT_8_GPIO 0 // TBD
#define IR_FRONT_9_GPIO 0 // TBD
#define IR_BACK_0_GPIO 0 // TBD
#define IR_BACK_1_GPIO 0 // TBD

#define LOAD_CELL_GPIO ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use

/* Semaphores and Mutexes */
SemaphoreHandle_t screen_mutex;

/* Tasks */
TaskHandle_t test_handle = NULL;
TaskHandle_t test_handle2 = NULL;
TaskHandle_t monitor_handle = NULL;

/* Timer handle */
gptimer_handle_t timer = NULL;

/* ADC Handles */
adc_oneshot_unit_handle_t adc1_handle;

/* Prototypes */
void init_ultrasonic(void);
void init_adc(void);
void init_pwm(int, int);

void pwm_start();
void pwm_stop();


/*Global variables*/
uint8_t IR_CHANNELS[] = {0, 1, 3, 4, 5, 6, 7, 8, 9};
int inf_values[6];

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

    /* Init functions */
    init_adc();
    init_pwm(M_MOTOR, M_MOTOR_GPIO);
    init_pwm(L_MOTOR, L_MOTOR_GPIO);
    init_pwm(R_MOTOR, R_MOTOR_GPIO);

    // Testing
    int adc_value = 0;
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1);


    while (1)
    {
        ESP_LOGI(main_name, "Main loop...");
        vTaskDelay(150 / portTICK_PERIOD_MS);
        
        pwm_start(M_MOTOR, 250);
        pwm_start(L_MOTOR, 250);
        pwm_start(R_MOTOR, 250);

        vTaskDelay(5000 / portTICK_PERIOD_MS);

        pwm_stop(M_MOTOR);
        pwm_stop(L_MOTOR);
        pwm_stop(R_MOTOR);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        /*
        xSemaphoreTake(screen_mutex, portMAX_DELAY);
        ESP_LOGI(main_name, "this is a task");
        xSemaphoreGive(screen_mutex);
        */
        //Code to be removed
        adc_oneshot_read(adc1_handle, IR_CHANNELS[0], &adc_value);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        ESP_LOGI(main_name, "ADC value: %d", adc_value);
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
}

/* Functions */
void init_adc(void)
{
    // Setting up ADC handle
    adc_oneshot_unit_init_cfg_t init_config1={
        .unit_id=ADC_UNIT_1, // ADC 1 is used
        .ulp_mode=ADC_ULP_MODE_DISABLE // Low power mode disabled
    }; // Default clock
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle)); // Error checking

    // Setting up ADC channels
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Default bitwidth
        .atten = ADC_ATTEN_DB_11, // Input attenuated, range increase by 11 dB
    };
    // Testing code:
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, 0, &config));
    // Actual code:
    /*
    for (int i = 0; i < ADC1_CHANNELS_NUM; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_CHANNELS[i], &config)); // Error checking
    }
    */
    /* Usage */
    // adc_oneshot_read(adc1_handle, IR_CHANNELS[0], &adc_value);    
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

/* Check each adc value of infrared sensor */
void infrared_adc_check(void)
{
    for (;;)
    {
        for (int i = 0; i < 5; i++)
        {
            adc_oneshot_read(adc1_handle, IR_CHANNELS[i], &inf_values[i]);
        }
    }
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

