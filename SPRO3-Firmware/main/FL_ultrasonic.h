#ifndef FL_ULTRASONIC_H
#define FL_ULTRASONIC_H
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


// Timer Macros
#define TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us

// Ultrasonic Macros
#define TRIG_PIN 
#define ECHO_PIN 
#define TRIG_PIN_1 1
#define ECHO_PIN_1 1
#define TRIG_PIN_2 1
#define ECHO_PIN_2 1

#define SOUND_SPEED_IN_US_PER_CM 58.0 // Speed of sound in microseconds per cm



/* Timer handle */
gptimer_handle_t timer = NULL;



void init_ultrasonic(void)
{
    gpio_set_direction(ECHO_PIN_1, GPIO_MODE_INPUT);
    gpio_set_direction(TRIG_PIN_1, GPIO_MODE_OUTPUT);

    gpio_set_direction(ECHO_PIN_2, GPIO_MODE_INPUT);
    gpio_set_direction(TRIG_PIN_2, GPIO_MODE_OUTPUT);

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


double distance_ultrasonic_1(void){

    double distance_obs;
    // Send a 10us pulse to the sensor's TRIG pin
    gpio_set_level(TRIG_PIN_1, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN_1, 0);

    ESP_LOGI("DisFunc", "Triggered");
    // Wait for the signal to arrive to the Echo pin
    while (gpio_get_level(ECHO_PIN_1) == 0);
    
    // Start timer 
    gptimer_start(timer);
    ESP_LOGI("DisFunc", "Started timer!");
    
    // While the pin Echo is high
    while (gpio_get_level(ECHO_PIN_1) == 1);

    // Stop timer
    gptimer_stop(timer);

    // Time that the pin ECHO is high 
    uint64_t time_Echo_High;  // positive
    gptimer_get_raw_count(timer, &time_Echo_High);  
    gptimer_set_raw_count(timer, 0); // reset timer

    // Distance in cm
    distance_obs = (double)time_Echo_High / SOUND_SPEED_IN_US_PER_CM;

    ESP_LOGI("DisFunc1", "DisFunc %f", distance_obs);

    return distance_obs;
}

double distance_ultrasonic_2(void){

    double distance_obs;
    // Send a 10us pulse to the sensor's TRIG pin
    gpio_set_level(TRIG_PIN_2, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN_2, 0);

    ESP_LOGI("DisFunc", "Triggered");
    // Wait for the signal to arrive to the Echo pin
    while (gpio_get_level(ECHO_PIN_2) == 0);
    
    // Start timer 
    gptimer_start(timer);
    ESP_LOGI("DisFunc", "Started timer!");
    
    // While the pin Echo is high
    while (gpio_get_level(ECHO_PIN_2) == 1);

    // Stop timer
    gptimer_stop(timer);

    // Time that the pin ECHO is high 
    uint64_t time_Echo_High;  // positive
    gptimer_get_raw_count(timer, &time_Echo_High);  
    gptimer_set_raw_count(timer, 0); // reset timer

    // Distance in cm
    distance_obs = (double)time_Echo_High / SOUND_SPEED_IN_US_PER_CM;

    ESP_LOGI("DisFunc2", "DisFunc %f", distance_obs);

    return distance_obs;
}