#include <stdio.h>
#include <string.h>

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

//#include "FL_endstop.h"

#define CALIBRATION_BLACK_TAPE 2000
#define CALIBRATION_FLOOR_D1_BACK
#define CALIBRATION_FLOOR_D2_BACK
#define CALIBRATION_FLOOR_D3_BACK
#define CALIBRATION_BLACK_D4_FRONT 2000
#define CALIBRATION_BLACK_D5_FRONT 900
#define CALIBRATION_FLOOR_D6_BACK
#define CALIBRATION_FLOOR_D7_BACK
#define CALIBRATION_FLOOR_D8_BACK
#define CALIBRATION_FLOOR
#define BORDER_VALUE

#define IR_D1 0
#define IR_D2 1
#define IR_D3 2
#define IR_D4 3
#define IR_D5 4
#define IR_D6 5
#define IR_D7 6
#define IR_D8 7

#define BASE_SPEED_L 150
#define BASE_SPEED_R 150
#define ROTATE_LIGHT 165
#define ROTATE_STRONG 150

#define STRAIGHT BASE_SPEED_L , BASE_SPEED_R
#define REVERSE -BASE_SPEED_L , -BASE_SPEED_R
#define STOP 0 , 0

#define RIGHT_TURN_LIGHT 130 , 105
#define RIGHT_TURN_LIGHT_REV -130 , -105
#define RIGHT_TURN_STRONG 200 , 80
#define RIGHT_ROTATE_LIGHT ROTATE_LIGHT , -ROTATE_LIGHT
#define RIGHT_ROTATE_STRONG ROTATE_STRONG , -ROTATE_STRONG

#define LEFT_TURN_LIGHT 105 , 130
#define LEFT_TURN_LIGHT_REV -105 , -130
#define LEFT_TURN_STRONG 80 , 200
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

#define LEFT -1
#define STEP 0
#define RIGHT 1
#define END 2


#define ENDSTOP_UP GPIO_NUM_15
#define ENDSTOP_DOWN GPIO_NUM_22


int turns = 0;
int decision[10] = {END};

int X_c = 1; // Entry point X
int Y_c = 1; // Entry point Y
int X_t = 1;
int Y_t = 1;

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

void init_endstop(void)
{
    gpio_reset_pin(ENDSTOP_UP);
    gpio_intr_disable(ENDSTOP_UP);
    gpio_set_direction(ENDSTOP_UP, GPIO_MODE_INPUT);
    gpio_pullup_en(ENDSTOP_UP);

    gpio_reset_pin(ENDSTOP_DOWN);
    gpio_intr_disable(ENDSTOP_DOWN);
    gpio_set_direction(ENDSTOP_DOWN, GPIO_MODE_INPUT);
    gpio_pullup_en(ENDSTOP_DOWN);
}

int check_endstop_up(void)
{
    return gpio_get_level(ENDSTOP_UP);
}

int check_endstop_down(void)
{
    return gpio_get_level(ENDSTOP_DOWN);
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

void step(void)
{
    pwm_drive(STRAIGHT);
    vTaskDelay(250 / portTICK_PERIOD_MS);

    while(!((ir_values_front[IR_D1] > 2000) && (ir_values_front[IR_D8] > 700)))
    {   
        xSemaphoreTake(ir_monitor_mutex, portMAX_DELAY);

        if((ir_values_back[IR_D4] > CALIBRATION_BLACK_TAPE) || (ir_values_back[IR_D5] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Straight");
            pwm_drive(STRAIGHT);
        }
        else if ((ir_values_back[IR_D3] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Right Light");
            pwm_drive(RIGHT_TURN_LIGHT);
        }
        else if ((ir_values_back[IR_D6] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Left Light");
            pwm_drive(LEFT_TURN_LIGHT);
        }

        ir_sensor_put_web();

        xSemaphoreGive(ir_monitor_mutex);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    pwm_drive(STOP);
}

void step_back(void)
{
    pwm_drive(REVERSE);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while(!((ir_values_front[IR_D1] > 2000) && (ir_values_front[IR_D8] > 700)))
    {   
        xSemaphoreTake(ir_monitor_mutex, portMAX_DELAY);

        if((ir_values_back[IR_D4] > CALIBRATION_BLACK_TAPE) || (ir_values_back[IR_D5] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Reverse");
            pwm_drive(REVERSE);
        }
        else if ((ir_values_back[IR_D6] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Right Light");
            pwm_drive(RIGHT_TURN_LIGHT_REV);
        }
        else if ((ir_values_back[IR_D3] > CALIBRATION_BLACK_TAPE))
        {
            strcpy(turn_decision, "Left Light");
            pwm_drive(LEFT_TURN_LIGHT_REV);
        }

        ir_sensor_put_web();

        xSemaphoreGive(ir_monitor_mutex);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    pwm_drive(STOP);
}

void turn(int direction)
{
    if(direction == LEFT)
    {
        pwm_drive(LEFT_ROTATE_LIGHT);
    }
    else
    {
        pwm_drive(RIGHT_ROTATE_LIGHT);
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);

    while(!((ir_values_back[IR_D4] > CALIBRATION_BLACK_TAPE) || (ir_values_back[IR_D5] > CALIBRATION_BLACK_TAPE)))
    {
        strcpy(turn_decision, "Intersection");
        ir_sensor_put_web();

        if(direction == LEFT)
        {
            pwm_drive(LEFT_ROTATE_LIGHT);
        }
        else
        {
            pwm_drive(RIGHT_ROTATE_LIGHT);
        }
            
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } 

    pwm_drive(STRAIGHT);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    pwm_drive(STOP);

}

void drive_down_fork(void)
{
    direction_set(M_MOTOR, DOWNWARD);
    pwm_set(M_MOTOR, 250);
    while (!(check_endstop_down() == 0))
    {        
        if(check_endstop_down() == 0) 
        {
            vTaskDelay(275 / portTICK_PERIOD_MS);
            pwm_set(M_MOTOR, 0);
            break;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
}

void drop_pallet(int x_target, int y_target)
{
    step();
    step();
    turn(LEFT);

    for (int i = 0; i < (y_target -1); i++)
    {
        step();
    }

    if (x_target == 1)
    {
        turn(RIGHT);
    }
    else
    {
        turn(LEFT);
    }    
    step_back();
    step_back();
    
    drive_down_fork();

    step();
    
    direction_set(M_MOTOR, UPWARD);
    pwm_set(M_MOTOR, 250);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    pwm_set(M_MOTOR, 0);
    
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