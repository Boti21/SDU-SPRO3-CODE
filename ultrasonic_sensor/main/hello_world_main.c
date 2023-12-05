
#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>

#include "driver/timer.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define TRIG_PIN 19
#define ECHO_PIN 18
#define SOUND_SPEED_IN_US_PER_CM 58.0 // Speed of sound in microseconds per cm

/* Misc macros */
#define CUSTOM_STACK_SIZE 2048
#define TIMER_RESOLUTION 1000000 // 1MHz, 1 tick = 1us

/* Timer handle */
gptimer_handle_t timer = NULL;

const TickType_t xDelay10uS = pdMS_TO_TICKS(1);

void init_ultrasonic(void);
double distance_ultrasonic(void);

double distance_obs;

void app_main(void)
{
    vTaskDelay(10);
    for(;;){
        vTaskDelay(xDelay10uS);
        printf("Hello, I am alive!");
        init_ultrasonic();
        distance_ultrasonic();

        vTaskDelay(pdMS_TO_TICKS(10));

    }
   

}

void init_ultrasonic(void) {

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
    ESP_ERROR_CHECK(gptimer_enable(timer));
    printf("Hello, I am alive from timerinit!");
}
double distance_ultrasonic(void){

    // Send a 10us pulse to the sensor's TRIG pin
    gpio_set_level(TRIG_PIN, 1);
    vTaskDelay(xDelay10uS);
    gpio_set_level(TRIG_PIN, 0);

    // wait for the signal to arrive to the Echo pin
    while (gpio_get_level(ECHO_PIN) == 0){

        // do nothing
    }

    // Start timer 
    gptimer_start(timer);

    // While the pin Echo is high
    while (gpio_get_level(ECHO_PIN) == 1){

        //  do nothing

    }

    // Stop timer
    gptimer_stop(timer);

    // Time that the pin ECHO is high 
    uint64_t time_Echo_High;  // positive
  gptimer_get_raw_count(timer, &time_Echo_High);  

  gptimer_set_raw_count(timer, 0); // reset timer

    // Distance in cm
    distance_obs = (double)time_Echo_High / SOUND_SPEED_IN_US_PER_CM;

    printf("\nDistance: %f", distance_obs);

    return distance_obs;

        
}