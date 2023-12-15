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

    bool flip = false;

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
        
        //display_weight(1234);
        
        ESP_LOGI(main_name, "Main loop...");
        //vTaskDelay(250 / portTICK_PERIOD_MS);

        battery_read();
        loadcell_read();
        

        /*
        pwm_drive(STRAIGHT);

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        pwm_drive(RIGHT_ROTATE_STRONG);

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        pwm_drive(LEFT_ROTATE_STRONG);
       
        */
        //pwm_drive(STRAIGHT);

        // Read IR-SENSOR in the front and the back
        ir_adc_multiplexer_check_front();
        //vTaskDelay(10 / portTICK_PERIOD_MS);
        ir_adc_multiplexer_check_back();
        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        ir_sensor_put_web();
        
        //ESP_LOGI(main_name, "\n");
        //vTaskDelay(2500 / portTICK_PERIOD_MS);
        //ir_adc_multiplexer_check_back();
        
        while (!((ir_values_back[IR_D1] > CALIBRATION_BLACK_TAPE) && (ir_values_back[IR_D8] > CALIBRATION_BLACK_TAPE))) // While the outer sensors are not on black at the same time
        {  

            if(ir_values_back[IR_D1] > CALIBRATION_BLACK_TAPE) 
            {
                pwm_drive(LEFT_TURN_STRONG);

            } else if (ir_values_back[IR_D8] > CALIBRATION_BLACK_TAPE)
            {
                pwm_drive(RIGHT_TURN_STRONG);

            } else if ((ir_values_front[IR_D4] > CALIBRATION_BLACK_D4_FRONT) && (ir_values_front[IR_D5] > CALIBRATION_BLACK_D5_FRONT) && (ir_values_back[IR_D4] > CALIBRATION_BLACK_TAPE) && (ir_values_back[IR_D5] > CALIBRATION_BLACK_TAPE))
            {
                pwm_drive(STRAIGHT);

            } else if (ir_values_front[IR_D4] < CALIBRATION_BLACK_TAPE)
            {
                pwm_drive(RIGHT_TURN_LIGHT);

            } else if (ir_values_front[IR_D5] < CALIBRATION_BLACK_TAPE)
            {
                pwm_drive(LEFT_TURN_LIGHT);
                
            } else
            pwm_drive(STRAIGHT);
            
            
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
        
        //pwm_drive(STOP);

        //pwm_drive(RIGHT_ROTATE_STRONG);

        vTaskDelay(300 / portTICK_PERIOD_MS);
        
        /*
        for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {

            ESP_LOGI("IR_RESULTS_FRONT", "Val %d: %d", i, ir_values_front[i]);
        
        }
        
        
        for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {

            ESP_LOGI("IR_RESULTS_BACK", "Val %d: %d", i, ir_values_back[i]);
        
        }
        */
        

        // Giving the operating system room to breath
        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        

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
        