
#include <stdio.h>
#include "ledc.h"
#include "esp_err.h"

#define Mast_motor  0
#define Left_motor  1
#define Right_motor 2

#define Mast_motor_GPIO 5
#define Left_motor_GPIO  18
#define Right_motor_GPIO 19

void pwm_start();
void pwm_stop();

static void pwm_init(int motor, int GPIO)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = 0, // Low speed mode
        .duty_resolution  = 8,// resolution in bit, 
        .timer_num        = motor, // timer x
        .freq_hz          = 1000,// Hz
        .clk_cfg          = 0 // configures clock automatically 
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = 0, // Low speed mode 
        .channel        = motor,
        .timer_sel      = motor,
        .intr_type      = 0, // disable interrupts
        .gpio_num       = GPIO,
        .duty           = 128, //Set duty to 50% in 8 bit :  (2**8)*50% = 128
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    ledc_timer_pause(0,motor);
}

void app_main(void)
{
    //initalise pwm
    pwm_init(Mast_motor,Mast_motor_GPIO);
    pwm_init(Left_motor,Left_motor_GPIO);
    pwm_init(Right_motor,Right_motor_GPIO);
    //speed control 0-255
    pwm_start(Mast_motor,250);
    pwm_start(Left_motor,200);
    pwm_start(Right_motor,40);

    pwm_stop(Mast_motor);
    pwm_stop(Left_motor);
    pwm_stop(Right_motor);
}

void pwm_start(int motor, int duty){
    ledc_set_duty(0,motor,duty);   
    ledc_timer_resume(0,motor);
}
void pwm_stop(int motor){
   ledc_timer_pause(0,motor);
}