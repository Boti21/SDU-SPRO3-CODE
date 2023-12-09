#ifndef FL_ADC_IR_H
#define FL_ADC_IR_H
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


// #define CONFIG_IDF_TARGET_ESP32S2
#define ADC1_0 0 // GPIO_NUM_36
#define ADC1_3 3 // GPIO_NUM_39
#define ADC1_6 6 // GPIO_NUM_34
#define ADC1_7 7 // GPIO_NUM_35
#define ADC1_4 4 // GPIO_NUM_32
#define ADC1_5 5 // GPIO_NUM_33
#define ADC1_8 8 // GPIO_NUM_25
#define ADC1_9 9 // GPIO_NUM_26
#define ADC2_0 0 // GPIO_NUM_4
#define ADC2_2 2 // GPIO_NUM_2
#define ADC2_3 3 // GPIO_NUM_15

#define IR_FRONT_NUMBER_OF_PINS 4
#define IR_BACK_NUMBER_OF_PINS 2

#define IR_FRONT_D1_GPIO ADC1_0 // GPIO_NUM_36
#define IR_FRONT_D3_GPIO ADC1_3 // GPIO_NUM_39
#define IR_FRONT_D4_GPIO ADC1_6 // GPIO_NUM_34
#define IR_FRONT_D5_GPIO ADC1_7 // GPIO_NUM_35
#define IR_FRONT_D6_GPIO ADC1_4 // GPIO_NUM_32
#define IR_FRONT_D8_GPIO ADC1_5 // GPIO_NUM_33
#define IR_BACK_D4_GPIO ADC1_8 // GPIO_NUM_25
#define IR_BACK_D5_GPIO ADC1_9 // GPIO_NUM_26
#define LOAD_CELL_GPIO ADC2_0

#define CORRECTION_NONE 0 // Could be changed obviously
#define CORRECTION_LEFT -1 // Could be changed obviously
#define CORRECTION_RIGHT 1 // Could be changed obviously
#define CORRECTION_LIGHT 10 // No idea if this will be enough
#define CORRECTION_HARD 50 // No idea if this will be enough

#define IR_FRONT_LEFT_SENSOR 1
#define IR_FRONT_RIGHT_SENSOR 2

#define IR_BACK_LEFT_SENSOR 0
#define IR_BACK_RIGHT_SENSOR 1

adc_oneshot_unit_handle_t adc1_handle;

uint8_t IR_CHANNELS_FRONT[] = {IR_FRONT_D1_GPIO, IR_FRONT_D4_GPIO, IR_FRONT_D5_GPIO, IR_FRONT_D8_GPIO};
int ir_values_front[IR_FRONT_NUMBER_OF_PINS];

uint8_t IR_CHANNELS_BACK[] = {IR_BACK_D4_GPIO, IR_BACK_D5_GPIO};
int ir_values_back[IR_BACK_NUMBER_OF_PINS];

/* Data handling variables */
unsigned int max_front = 0;
unsigned int min_front = 0;
unsigned int dif_front = 0;
unsigned int threshhold_front = 0;

unsigned int max_back = 0;
unsigned int min_back = 0;
unsigned int dif_back = 0;
unsigned int threshhold_back = 0;

typedef struct {
    int correction_dir;
    int correction_level;
} ir_check_line_ret;

typedef enum {
    NO_INTERSECTION,
    INTERSECTION,
} intersection_t;

intersection_t grid_intersection = NO_INTERSECTION;

// Use this to check the intersection variable
SemaphoreHandle_t intersection_mutex = NULL;

void init_adc(void)
{
    // Setting up ADC handle
    adc_oneshot_unit_init_cfg_t init_config1={
        .unit_id = ADC_UNIT_1, // ADC 1 is used
        .ulp_mode = ADC_ULP_MODE_DISABLE // Low power mode disabled
    }; // Default clock
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle)); // Error checking

    // Setting up ADC channels
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Default bitwidth
        .atten = ADC_ATTEN_DB_11, // Input attenuated, range increase by 11 dB
    };
    // Actual code:
    for (int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_CHANNELS_FRONT[i], &config)); // Error checking
    }
    /* Usage */
    // adc_oneshot_read(adc1_handle, IR_CHANNELS[0], &adc_value);    
}

/* Check each adc value of infrared sensor */
void ir_adc_check_front(void)
{
    for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++)
    {
        adc_oneshot_read(adc1_handle, IR_CHANNELS_FRONT[i], &ir_values_front[i]);
    }
}

void ir_adc_check_back(void)
{
    for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++)
    {
        adc_oneshot_read(adc1_handle, IR_CHANNELS_BACK[i], &ir_values_back[i]);
    }
}

void ir_max_front(void) {
    for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {
        if(max_front < ir_values_front[i]) {
            max_front = ir_values_front[i];
        }
    }
}

void ir_min_front(void) {
    for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++) {
        if(min_front > ir_values_front[i]) {
            min_front = ir_values_front[i];
        }
    }
}

void ir_dif_front(void) {
    dif_front = max_front - min_front;
}

void ir_threshhold_front(void) {
    threshhold_front = (max_front + min_front) / 2;
}


void ir_max_back(void) {
    for(int i = 0; i < IR_BACK_NUMBER_OF_PINS; i++) {
        if(max_back < ir_values_back[i]) {
            max_back = ir_values_back[i];
        }
    }
}

void ir_min_back(void) {
    for(int i = 0; i < IR_BACK_NUMBER_OF_PINS; i++) {
        if(min_back > ir_values_back[i]) {
            min_back = ir_values_back[i];
        }
    }
}

void ir_dif_back(void) {
    dif_back = max_back - min_back;
}

void ir_threshhold_back(void) {
    threshhold_back = (max_back + min_back) / 2;
}

ir_check_line_ret ir_check_line_front(void)
{
    ir_check_line_ret result = {
        .correction_dir = CORRECTION_NONE,
        .correction_level = CORRECTION_NONE,
    };

    ir_adc_check_front();
    ir_max_front();
    ir_min_front();
    ir_dif_front();
    ir_threshhold_front();


    if(ir_values_front[IR_FRONT_LEFT_SENSOR] < threshhold_front)
    {
        result.correction_dir = CORRECTION_RIGHT;
    }
    else if(ir_values_front[IR_FRONT_RIGHT_SENSOR] < threshhold_front)
    {
        result.correction_dir = CORRECTION_LEFT;
    }

    result.correction_level = CORRECTION_LIGHT;

    return result;
}

