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

// ForkLift includes
#include "FL_ultrasonic.h"
#include "FL_drive.h"
#include "FL_ADCandIR.h"
#include "FL_threads.h"


#define CALIBRATION_BLACK_TAPE 2000
#define CALIBRATION_FLOOR
#define BORDER_VALUE

#define BASE_SPEED 150
#define ROTATE_LIGHT 90
#define ROTATE_STRONG 150

#define STRAIGHT BASE_SPEED, BASE_SPEED

#define RIGHT_TURN_LIGHT 250 , 150
#define RIGHT_TURN_STRONG 250 , 100
#define RIGHT_ROTATE_LIGHT ROTATE_LIGHT , -ROTATE_LIGHT
#define RIGHT_ROTATE_STRONG ROTATE_STRONG , -ROTATE_STRONG

#define LEFT_TURN_LIGHT 150 , 250
#define LEFT_TURN_STRONG 100 , 250
#define LEFT_ROTATE_LIGHT -ROTATE_LIGHT , ROTATE_LIGHT
#define LEFT_ROTATE_STRONG -ROTATE_STRONG , ROTATE_STRONG

#define LOAD_CELL_GPIO ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use


void app_main(void)
{
    /* Initializing mutexes and semaphores */
    screen_mutex = xSemaphoreCreateMutex();


    /* Little boot up message ;) */ // <-- very Boti comment  // still not sure if that is good
    char *main_name = pcTaskGetName(NULL);     // A way to get the name of the current task
    ESP_LOGI(main_name, "Program started..."); // The way to print something to the terminal

    /* Task creation */
    // xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    // xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);
    // xTaskCreate(monitor_task, "monitor_task", CUSTOM_STACK_SIZE, NULL, 2, &monitor_handle);

    /* Init functions */
    init_adc();
    init_pwm(M_MOTOR, M_MOTOR_GPIO);
    init_pwm(L_MOTOR, L_MOTOR_GPIO);
    init_pwm(R_MOTOR, R_MOTOR_GPIO);
    //init_ultrasonic();

    init_direction_change();
    
    /*
    // Testing
    int adc_value = 0;
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1);
    */


    while (1) {
        
        ESP_LOGI(main_name, "Main loop...");
        //vTaskDelay(250 / portTICK_PERIOD_MS);

        
       
        
        // Read IR-SENSOR in the front
        ir_adc_check_front();

        if(ir_values_front[IR_D1] > CALIBRATION_BLACK_TAPE) 
        {
            pwm_drive(LEFT_TURN_STRONG);

        } else if (ir_values_front[IR_D8] > CALIBRATION_BLACK_TAPE)
        {
            pwm_drive(RIGHT_TURN_STRONG);

        } else if ((ir_values_front[IR_D4] > CALIBRATION_BLACK_TAPE) && (ir_values_front[IR_D5] > CALIBRATION_BLACK_TAPE))
        {
            pwm_drive(STRAIGHT);

        } else if (ir_values_front[IR_D4] < CALIBRATION_BLACK_TAPE)
        {
            pwm_drive(RIGHT_TURN_LIGHT);

        } else if (ir_values_front[IR_D5] < CALIBRATION_BLACK_TAPE)
        {
            pwm_drive(LEFT_TURN_LIGHT);
            
        }
        
        
        for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {

            ESP_LOGI("IR_RESULTS_FRONT", "Val %d: %d", i, ir_values_front[i]);
        
        }
        

        // Giving the operating system room to breath
        vTaskDelay(100 / portTICK_PERIOD_MS);

        

    }
}






//vTaskDelay(5000 / portTICK_PERIOD_MS);
        /*
        pwm_stop(M_MOTOR);
        pwm_stop(L_MOTOR);
        pwm_stop(R_MOTOR);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        */

/* Example of using a mutex
xSemaphoreTake(screen_mutex, portMAX_DELAY);
ESP_LOGI(main_name, "this is a task");
xSemaphoreGive(screen_mutex);
*/
        