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

#define CALIBRATION_BLACK_TAPE 2000
#define CALIBRATION_FLOOR_D1_BACK
#define CALIBRATION_FLOOR_D2_BACK
#define CALIBRATION_FLOOR_D3_BACK
#define CALIBRATION_BLACK_D4_FRONT 2000
#define CALIBRATION_BLACK_D5_FRONT 500
#define CALIBRATION_FLOOR_D6_BACK
#define CALIBRATION_FLOOR_D7_BACK
#define CALIBRATION_FLOOR_D8_BACK
#define CALIBRATION_FLOOR
#define BORDER_VALUE

#define BASE_SPEED 150
#define ROTATE_LIGHT 90
#define ROTATE_STRONG 150

#define STRAIGHT BASE_SPEED , BASE_SPEED
#define STOP 0 , 0

#define RIGHT_TURN_LIGHT 175 , 125
#define RIGHT_TURN_STRONG 175 , 100
#define RIGHT_ROTATE_LIGHT ROTATE_LIGHT , -ROTATE_LIGHT
#define RIGHT_ROTATE_STRONG ROTATE_STRONG , -ROTATE_STRONG

#define LEFT_TURN_LIGHT 125 , 175
#define LEFT_TURN_STRONG 100 , 175
#define LEFT_ROTATE_LIGHT -ROTATE_LIGHT , ROTATE_LIGHT
#define LEFT_ROTATE_STRONG -ROTATE_STRONG , ROTATE_STRONG

#define M_MOTOR 0 // Mast motor
#define L_MOTOR 1 // Left motor (facing the same way as the fork)
#define R_MOTOR 2 // Right motor (facing the same way as the fork)
#define M_MOTOR_GPIO 5 // TBD
#define L_MOTOR_GPIO 19 // TBD
#define R_MOTOR_GPIO 18 // TBD 

#define R_MOTOR_FORWARD GPIO_NUM_17
#define R_MOTOR_BACKWARD GPIO_NUM_0 // prev 23
#define L_MOTOR_FORWARD GPIO_NUM_2
#define L_MOTOR_BACKWARD GPIO_NUM_4
#define M_MOTOR_FORWARD GPIO_NUM_13
#define M_MOTOR_BACKWARD GPIO_NUM_16

// Note
// Number 15 will always be high! - Even if code says otherwise

#define BACKWARD 0
#define FORWARD 1
#define DOWNWARD 0
#define UPWARD 1

void pwm_set(int motor, int duty){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor, duty);  
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor); 
    ledc_timer_resume(LEDC_LOW_SPEED_MODE, motor);
}

void init_pwm(int motor, int GPIO)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE, // Low speed mode
        .duty_resolution  = 8,// resolution in bit, 
        .timer_num        = 0, // timer x
        .freq_hz          = 1000,// Hz // prev 1000
        .clk_cfg          = 0 // configures clock automatically 
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE, // Low speed mode 
        .channel        = motor,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = 0, // disable interrupts
        .gpio_num       = GPIO,
        .duty           = 128, //Set duty to 50% in 8 bit :  (2**8)*50% = 128
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    //ledc_timer_pause(0,motor);

    pwm_set(motor, 0);
   // gpio_set_direction(GPIO, GPIO_MODE_OUTPUT);
}




void pwm_stop(int motor){

   ledc_timer_pause(0, motor);
}


void init_direction_change(void)
{
    gpio_reset_pin(L_MOTOR_FORWARD);
    gpio_reset_pin(R_MOTOR_FORWARD);
    gpio_reset_pin(M_MOTOR_FORWARD);
    gpio_reset_pin(L_MOTOR_BACKWARD);
    gpio_reset_pin(R_MOTOR_BACKWARD);
    gpio_reset_pin(M_MOTOR_BACKWARD);

    gpio_set_direction(L_MOTOR_FORWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(R_MOTOR_FORWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(M_MOTOR_FORWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(L_MOTOR_BACKWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(R_MOTOR_BACKWARD, GPIO_MODE_OUTPUT);
    gpio_set_direction(M_MOTOR_BACKWARD, GPIO_MODE_OUTPUT);

    gpio_set_level(L_MOTOR_FORWARD, 1);
    gpio_set_level(R_MOTOR_FORWARD, 1);
    gpio_set_level(M_MOTOR_FORWARD, 1);

    gpio_set_level(L_MOTOR_BACKWARD, 0);
    gpio_set_level(R_MOTOR_BACKWARD, 0);
    gpio_set_level(M_MOTOR_BACKWARD, 0);
}

void direction_set(int motor, int direction)
{ 
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
    } else if (motor == M_MOTOR) 
    {
        if (direction == FORWARD)
        {
            gpio_set_level(M_MOTOR_FORWARD, 1);
            gpio_set_level(M_MOTOR_BACKWARD, 0);
        }
        else
        {
            gpio_set_level(M_MOTOR_FORWARD, 0);
            gpio_set_level(M_MOTOR_BACKWARD, 1);
        }
    }
}

void pwm_drive(int duty_L, int duty_R)
{
    if (duty_L < 0) 
    {
        direction_set(L_MOTOR, BACKWARD);
    }
    else
    {
        direction_set(L_MOTOR, FORWARD);
    }

    if (duty_R < 0)
    {
        direction_set(R_MOTOR, BACKWARD);
    }
    else
    {
        direction_set(R_MOTOR, FORWARD);
    }   

    pwm_set(L_MOTOR, abs(duty_L));
    pwm_set(R_MOTOR, abs(duty_R));
}


/*
void drive_intersections(int intersection_count) 
{
    int threshhold;
    
    ir_adc_check_front();
    ir_min();
    ir_max();
    threshhold = ir_avg();

    if ((ir_values_front[1] > threshhold) && (ir_values_front[2] > threshhold)) // Line is in center
    {
        
    }
    
}
*/