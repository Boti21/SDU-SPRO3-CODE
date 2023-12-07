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



#define M_MOTOR 0 // Mast motor
#define L_MOTOR 1 // Left motor (facing the same way as the fork)
#define R_MOTOR 2 // Right motor (facing the same way as the fork)
#define M_MOTOR_GPIO 5 // TBD
#define L_MOTOR_GPIO 18 // TBD
#define R_MOTOR_GPIO 19 // TBD

#define L_MOTOR_FORWARD GPIO_NUM_22
#define L_MOTOR_BACKWARD GPIO_NUM_23
#define R_MOTOR_FORWARD GPIO_NUM_2
#define R_MOTOR_BACKWARD GPIO_NUM_15

#define BACKWARD 0
#define FORWARD 1


void init_pwm(int motor, int GPIO)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = 0, // Low speed mode
        .duty_resolution  = 8,// resolution in bit, 
        .timer_num        = 0, // timer x
        .freq_hz          = 1000,// Hz // prev 1000
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

void pwm_set(int motor, int duty){
    ledc_set_duty(0,motor,duty);   
    ledc_timer_resume(0,motor);
}
void pwm_stop(int motor){
   ledc_timer_pause(0,motor);
}

void pwm_wheels(int duty_L, int duty_R) {
    pwm_set(L_MOTOR, duty_L);
    pwm_set(R_MOTOR, duty_R);
}

void init_direction_change(void) {

    gpio_set_direction(L_MOTOR_FORWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(R_MOTOR_FORWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(L_MOTOR_BACKWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(R_MOTOR_BACKWARD, GPIO_MODE_OUTPUT);

    gpio_set_level(L_MOTOR_FORWARD, 1);
    gpio_set_level(R_MOTOR_FORWARD, 1);

    gpio_set_level(L_MOTOR_BACKWARD, 0);
    gpio_set_level(R_MOTOR_BACKWARD, 0);

}

void direction_set(int motor, int direction) { 

    if (motor == L_MOTOR)
    {
        if (direction == FORWARD)
        {
            gpio_set_level(L_MOTOR_FORWARD, 1);
            gpio_set_level(L_MOTOR_BACKWARD, 0);
        }
        else
        {
            gpio_set_level(L_MOTOR_FORWARD, 0);
            gpio_set_level(L_MOTOR_BACKWARD, 1);
        }
        
    } 
    else if (motor == R_MOTOR) 
    {
        if (direction == FORWARD)
        {
            gpio_set_level(R_MOTOR_FORWARD, 1);
            gpio_set_level(R_MOTOR_BACKWARD, 0);
        }
        else
        {
            gpio_set_level(R_MOTOR_FORWARD, 0);
            gpio_set_level(R_MOTOR_BACKWARD, 1);
        }
    }

}