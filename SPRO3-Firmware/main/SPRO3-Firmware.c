// Note: If more pin are needed - use just three pinis for multiplexer addressing
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

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
#include "FL_ADC_IR.h"
#include "FL_threads.h"
#include "FL_fork_connect.h"

#ifndef FL_DISPLAY_H
#include "FL_display.h"
#endif

#define EVER ; ;

// Temporarily outcommented bc of build error
// #define LOAD_CELL_GPIO ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPIO we want to use


void app_main(void)
{
    /* Initializing mutexes and semaphores */
    screen_mutex = xSemaphoreCreateMutex();
    web_mutex = xSemaphoreCreateMutex();
    FL_events = xEventGroupCreate();


    /* Little boot up message ;) */ // <-- very Boti comment  // still not sure if that is good
    char *main_name = pcTaskGetName(NULL);     // A way to get the name of the current task
    ESP_LOGI(main_name, "Program started..."); // The way to print something to the terminal

    /* Task creation */
    // xTaskCreate(test_task, "test_task", CUSTOM_STACK_SIZE, NULL, 2, &test_handle);
    // xTaskCreate(test_task2, "test_task2", CUSTOM_STACK_SIZE, NULL, 2, &test_handle2);
    // xTaskCreate(monitor_task, "monitor_task", CUSTOM_STACK_SIZE, NULL, 2, &monitor_handle);

    /* Init functions */
    init_fork_connect();
    //strcpy(changing_text, "Hello");
    
    init_adc();
    init_multiplexer();
    
    init_pwm(M_MOTOR, M_MOTOR_GPIO);
    init_pwm(L_MOTOR, L_MOTOR_GPIO);
    init_pwm(R_MOTOR, R_MOTOR_GPIO);
    init_direction_change();
    
    init_ultrasonic();
    
    //init_display();
    //display_weight(1234);
    //display_voltage(3456);
    
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    pwm_drive(STRAIGHT);

    
    direction_set(M_MOTOR, UPWARD);
    //pwm_set(M_MOTOR, 255);

    for (EVER) 
    {        
        ESP_LOGI(main_name, "Main loop...");

        battery_read();
        loadcell_read();
       
        // Read IR-SENSOR in the front and the back
        ir_adc_multiplexer_check_front();
        //vTaskDelay(10 / portTICK_PERIOD_MS);
        ir_adc_multiplexer_check_back();
        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        ir_sensor_put_web();
        while (1)
        {
            if((ir_values_front[IR_D4] > CALIBRATION_BLACK_D4_FRONT) && (ir_values_front[IR_D5] > CALIBRATION_BLACK_D5_FRONT))
            {
                pwm_drive(STRAIGHT);
            }else if ((ir_values_front[IR_D4] > CALIBRATION_BLACK_D4_FRONT))
            {
                pwm_drive(LEFT_TURN_LIGHT);
            }
            else if ((ir_values_front[IR_D5] > CALIBRATION_BLACK_D5_FRONT))
            {
                pwm_drive(RIGHT_TURN_LIGHT);
            }

        }
    
       if ((ir_values_back[IR_D1] > CALIBRATION_BLACK_TAPE)){

       }
        //ESP_LOGI(main_name, "\n");
        //vTaskDelay(2500 / portTICK_PERIOD_MS);
        //ir_adc_multiplexer_check_back();
        
            vTaskDelay(10 / portTICK_PERIOD_MS);
            ir_adc_multiplexer_check_front();
            
            printf("\nFront:\n");
            isolate_line(ir_values_front);
            //vTaskDelay(10 / portTICK_PERIOD_MS);
            printf("\n");
            ir_adc_multiplexer_check_back();
            printf("\nBack:\n");
            isolate_line(ir_values_back);
            
        }
        
        if(check_endstop_up() == 1) {
            // Endstop reached
            direction_set(M_MOTOR, DOWNWARD);
        }
        else if (check_endstop_down == 1)
        {
            direction_set(M_MOTOR, UPWARD);
        }

        // Giving the operating system room to breath
        vTaskDelay(30 / portTICK_PERIOD_MS);        

}
