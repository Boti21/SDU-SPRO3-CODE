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

char turn_decision[20];

void app_main(void)
{
    /* Initializing mutexes and semaphores */
    screen_mutex = xSemaphoreCreateMutex();
    web_mutex = xSemaphoreCreateMutex();
    ir_monitor_mutex = xSemaphoreCreateMutex();
    load_cell_monitor_mutex = xSemaphoreCreateMutex();
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

    strcpy(turn_decision, "Undefined");
    
    init_adc();
    init_multiplexer();
    init_ultrasonic();
    init_display();
    
    init_pwm(M_MOTOR, M_MOTOR_GPIO);
    init_pwm(L_MOTOR, L_MOTOR_GPIO);
    init_pwm(R_MOTOR, R_MOTOR_GPIO);
    init_direction_change();
    

    xTaskCreatePinnedToCore (ir_sensor_monitor, //Function to implement the task
                            "ir_sensor_monitor", //Name of the task
                            3000, //Stack size in words
                            NULL, //Task input parameter
                            60, //Priority of the task
                            NULL, //Task handle.
                            APP_CPU_NUM); //Core where the task should run
    
    xTaskCreatePinnedToCore(loadcell_monitor,
                            "load_cell_monitor",
                            3000,
                            NULL,
                            tskIDLE_PRIORITY,
                            NULL,
                            PRO_CPU_NUM);
    
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    pwm_drive(STRAIGHT);

    direction_set(M_MOTOR, UPWARD);
    //pwm_set(M_MOTOR, 255);

    for (EVER) 
    {        
        ESP_LOGI(main_name, "Main loop...");

        turns = 0;
        
        while(turns < 8)
        {
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


            if(decision[turns] == LEFT)
            {
                pwm_drive(LEFT_ROTATE_LIGHT);
            }
            else
            {
                pwm_drive(RIGHT_ROTATE_LIGHT);
            }

            vTaskDelay(250 / portTICK_PERIOD_MS);

            while(!((ir_values_back[IR_D4] > CALIBRATION_BLACK_TAPE) || (ir_values_back[IR_D5] > CALIBRATION_BLACK_TAPE)))
            {
                strcpy(turn_decision, "Intersection");
                ir_sensor_put_web();

                if(decision[turns] == LEFT)
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

            turns++;
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS);  
    }
        
    // Giving the operating system room to breath
    vTaskDelay(30 / portTICK_PERIOD_MS);        
}
