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
#define ADC1_0 0 // GPIO_NUM_36 // Back ADC // prev front
#define ADC1_3 3 // GPIO_NUM_39 // Front ADC
#define ADC1_6 6 // GPIO_NUM_34
#define ADC1_7 7 // GPIO_NUM_35
#define ADC1_4 4 // GPIO_NUM_32
#define ADC1_5 5 // GPIO_NUM_33
#define ADC1_8 8 // GPIO_NUM_25
#define ADC1_9 9 // GPIO_NUM_26
#define ADC2_0 0 // GPIO_NUM_4
#define ADC2_2 2 // GPIO_NUM_2
#define ADC2_3 3 // GPIO_NUM_15

#define IR_FRONT_NUMBER_OF_PINS 8
#define IR_BACK_NUMBER_OF_PINS 8

#define IR_FRONT_D1_GPIO ADC1_0 // GPIO_NUM_36
#define IR_FRONT_D3_GPIO ADC1_3 // GPIO_NUM_39
#define IR_FRONT_D4_GPIO ADC1_6 // GPIO_NUM_34
#define IR_FRONT_D5_GPIO ADC1_7 // GPIO_NUM_35
#define IR_FRONT_D6_GPIO ADC1_4 // GPIO_NUM_32
#define IR_FRONT_D8_GPIO ADC1_5 // GPIO_NUM_33
#define IR_BACK_D4_GPIO ADC1_8 // GPIO_NUM_25
#define IR_BACK_D5_GPIO ADC1_9 // GPIO_NUM_26

#define IR_D1 0
#define IR_D2 1
#define IR_D3 2
#define IR_D4 3
#define IR_D5 4
#define IR_D6 5
#define IR_D7 6
#define IR_D8 7

#define LOAD_CELL_SLOPE 2625 // Slope of ideal line
#define LOAD_CELL_OFFSET -1050 // Offset of ideal line

/* Load cell ideal line
0.6 for 525 g
y = m*x + b
0 = m * 0.4 + b
525 = m * 0.6 + b

m = 2625
b = -1050

y = 2625 * x - 1050
*/

#define LOAD_CELL_GPIO ADC2_0

#define CORRECTION_NONE 0 // Could be changed obviously
#define CORRECTION_LEFT -1 // Could be changed obviously
#define CORRECTION_RIGHT 1 // Could be changed obviously
#define CORRECTION_LIGHT 10 // Could be completely unnecessary
#define CORRECTION_HARD 50 // Could be completely unnecessary

#define IR_FRONT_LEFT_SENSOR 1
#define IR_FRONT_RIGHT_SENSOR 2

#define IR_BACK_LEFT_SENSOR 0
#define IR_BACK_RIGHT_SENSOR 1

/* Multiplexer macros */
#define NUM_OF_ADDRESS_PINS 3

#define MULTIPLEXER2_A GPIO_NUM_32  
#define MULTIPLEXER2_B GPIO_NUM_23 
#define MULTIPLEXER2_C GPIO_NUM_25
 
#define MULTIPLEXER1_A GPIO_NUM_26
#define MULTIPLEXER1_B GPIO_NUM_27
#define MULTIPLEXER1_C GPIO_NUM_14

adc_oneshot_unit_handle_t adc1_handle;

uint8_t IR_CHANNELS_FRONT[] = {IR_FRONT_D3_GPIO, IR_FRONT_D4_GPIO, IR_FRONT_D5_GPIO, IR_FRONT_D6_GPIO};
int ir_values_front[IR_FRONT_NUMBER_OF_PINS];

uint8_t IR_CHANNELS_BACK[] = {IR_BACK_D4_GPIO, IR_BACK_D5_GPIO};
int ir_values_back[IR_BACK_NUMBER_OF_PINS];

uint8_t MULTIPLEXER_PINS[] = {MULTIPLEXER1_A, MULTIPLEXER1_B, MULTIPLEXER1_C, MULTIPLEXER2_A, MULTIPLEXER2_B, MULTIPLEXER2_C};
uint8_t multiplexer_adress[NUM_OF_ADDRESS_PINS] = {0};

char sensor_readings_string[150]; // Adjust the size as needed
extern SemaphoreHandle_t web_mutex;
extern char turn_decision[];


/* Data handling variables */
unsigned int max_front = 0;
unsigned int min_front = 0;
unsigned int dif_front = 0;
unsigned int threshhold_front = 0;

unsigned int max_back = 0;
unsigned int min_back = 0;
unsigned int dif_back = 0;
unsigned int threshhold_back = 0;
int read_battery_voltage = 0;

int loadcell_grams[10] = {0};

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
extern SemaphoreHandle_t ir_monitor_mutex;

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
    /* Setup with multiplexer */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_0, &config)); // Multiplexers
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_3, &config)); 
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_6, &config)); // Battery
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_7, &config)); // Loadcell
    /*
    for (int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_CHANNELS_FRONT[i], &config)); // Error checking
    }
    */

    /* Usage */
    // adc_oneshot_read(adc1_handle, IR_CHANNELS[0], &adc_value); 

}

void init_multiplexer(void)
{
    /* Not needed bc of ADC 
    gpio_reset_pin(GPIO_NUM_23);
    gpio_intr_disable(GPIO_NUM_23);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_NUM_23);
    
    gpio_reset_pin(GPIO_NUM_23);
    gpio_intr_disable(GPIO_NUM_23);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_NUM_23);
    */

    for(int i = 0; i < 6; i++) {
        gpio_reset_pin(MULTIPLEXER_PINS[i]);
        gpio_intr_disable(MULTIPLEXER_PINS[i]);
        gpio_set_direction(MULTIPLEXER_PINS[i], GPIO_MODE_OUTPUT);
        gpio_pulldown_en(MULTIPLEXER_PINS[i]);
    }
}

/* Functions for the multiplexer */
void dec_to_bin(int dec_num)
{
    for(int i = 0; i < NUM_OF_ADDRESS_PINS; i++) {
        multiplexer_adress[i] = 0;
    }
    
    for(int i = 0; i < NUM_OF_ADDRESS_PINS; i++) {
        int mask = 0b001;
        mask = mask << i;

        if(mask & dec_num) {
            multiplexer_adress[i] = 1;
        }
    }
}

/*
D1 working (3-0)
D2 working (3-0)
D3 working (3-0)
D4 working (3-0)
D5 working (5-0)
D6 working
D7 not really
D8 not wokring
*/

void set_multiplexer1_channel(int channel_num)
{
    dec_to_bin(channel_num);

    gpio_set_level(MULTIPLEXER1_A, 0);
    gpio_set_level(MULTIPLEXER1_B, 0);
    gpio_set_level(MULTIPLEXER1_C, 0);
    
    gpio_set_level(MULTIPLEXER1_A, multiplexer_adress[0]);
    gpio_set_level(MULTIPLEXER1_B, multiplexer_adress[1]);
    gpio_set_level(MULTIPLEXER1_C, multiplexer_adress[2]);

    // ESP_LOGI("MP1_channel", "Ch: %d Pins: %d %d %d", channel_num, multiplexer_adress[2], multiplexer_adress[1], multiplexer_adress[0]);
}

void set_multiplexer2_channel(int channel_num)
{
    dec_to_bin(channel_num);

    gpio_set_level(MULTIPLEXER2_A, multiplexer_adress[0]);
    gpio_set_level(MULTIPLEXER2_B, multiplexer_adress[1]);
    gpio_set_level(MULTIPLEXER2_C, multiplexer_adress[2]);
    //vTaskDelay(100 / portTICK_PERIOD_MS);

    // ESP_LOGI("MP2_channel", "Ch: %d Pins: %d %d %d", channel_num, multiplexer_adress[2], multiplexer_adress[1], multiplexer_adress[0]);
}

void ir_adc_multiplexer_check_back_thread(void) // prev front
{
    //printf("Back:\n");
    for(int i = 0; i < 8; i++) {
        set_multiplexer1_channel(i);
        vTaskDelay(2 / portTICK_PERIOD_MS);
        
        xSemaphoreTake(ir_monitor_mutex, portMAX_DELAY);
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_0, &ir_values_back[i]));
        xSemaphoreGive(ir_monitor_mutex);

        
        vTaskDelay(3 / portTICK_PERIOD_MS);
        
    }
    
    
}

void ir_adc_multiplexer_check_front_thread(void)  // prev back
{
    //printf("Front:\n");
    for(int i = 0; i < 8; i++) {
        set_multiplexer2_channel(7 - i); // To achieve that D1 is to the left in the array
        vTaskDelay(2 / portTICK_PERIOD_MS);

        xSemaphoreTake(ir_monitor_mutex, portMAX_DELAY);
        adc_oneshot_read(adc1_handle, ADC1_3, &ir_values_front[i]);
        xSemaphoreGive(ir_monitor_mutex);
        
        vTaskDelay(3 / portTICK_PERIOD_MS);
        
    }
    
}
void ir_adc_multiplexer_check_back(void) // prev front
{
    //printf("Back:\n");
    for(int i = 0; i < 8; i++) {
        set_multiplexer1_channel(i);

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_0, &ir_values_back[i]));
        vTaskDelay(12 / portTICK_PERIOD_MS);

        // ESP_LOGI("IR_RESULTS_FRONT", "Val %d: %d", i, ir_values_front[i]);
        //printf("%04d  ", ir_values_back[i]);
    }
    //printf("\n");
    
}

void ir_adc_multiplexer_check_front(void)  // prev back
{
    //printf("Front:\n");
    for(int i = 0; i < 8; i++) {
        set_multiplexer2_channel(7 - i); // To achieve that D1 is to the left in the array

        //vTaskDelay(20 / portTICK_PERIOD_MS);
        adc_oneshot_read(adc1_handle, ADC1_3, &ir_values_front[i]);
        // ESP_LOGI("IR_RESULTS_BACK", "Val %d: %d", i, ir_values_back[i]);
        vTaskDelay(12 / portTICK_PERIOD_MS);
        //printf("%04d  ", ir_values_front[i]);
    }
    //printf("\n");
}

void ir_sensor_put_web(void)
{

 xSemaphoreTake(web_mutex, portMAX_DELAY);
 
 sprintf(sensor_readings_string, "Front: %04d %04d %04d %04d %04d %04d %04d %04d\nBack: %04d %04d %04d %04d %04d %04d %04d %04d \n Decision: %s", 
        ir_values_front[0], ir_values_front[1], ir_values_front[2], ir_values_front[3], 
        ir_values_front[4], ir_values_front[5], ir_values_front[6], ir_values_front[7], 
        ir_values_back[0], ir_values_back[1], ir_values_back[2], ir_values_back[3], 
        ir_values_back[4], ir_values_back[5], ir_values_back[6], ir_values_back[7], turn_decision);

xSemaphoreGive(web_mutex);

}


// Check each adc value of infrared sensor 
void ir_adc_check_front(void)
{
    for(int i = 0; i < IR_FRONT_NUMBER_OF_PINS; i++)
    {
        adc_oneshot_read(adc1_handle, IR_CHANNELS_FRONT[i], &ir_values_front[i]);
    }
}

void ir_adc_check_back(void)
{
    for(int i = 0; i < IR_BACK_NUMBER_OF_PINS; i++)
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

void isolate_line(int* sensor_values)
{   
    
    for(int i = 0; i < 8; i++)
    {
        if (sensor_values[i] > CALIBRATION_BLACK_TAPE)
        {
            printf("   1  ");
        } else
            printf("   0  ");
        
    } 
    
    vTaskDelay(12 / portTICK_PERIOD_MS);    
    printf("\n");
}

int loadcell_read()
{
    float loadcell_voltage = 0;
    int loadcell_ADC;
    adc_oneshot_read(adc1_handle, ADC1_7, &loadcell_ADC);
    
    loadcell_voltage = ((float)loadcell_ADC * 2.4 / 4095.0);
    
    ESP_LOGI("LOADCELL:", "loadcell voltage: %f", loadcell_voltage);
    ESP_LOGI("LOADCELL:", "loadcell grams: %d", (int)((loadcell_voltage * LOAD_CELL_SLOPE) + LOAD_CELL_OFFSET));

    return (int)((loadcell_voltage * LOAD_CELL_SLOPE) + LOAD_CELL_OFFSET);
}

int battery_read()
{
    // battery voltage reading => Vout = Dout*Vmax/Dmax

    float battery_voltage = 0;

    adc_oneshot_read(adc1_handle, ADC1_6, &read_battery_voltage); // Dout = read_battery_voltage

    battery_voltage = ((float)read_battery_voltage * 2.4 / 4095.0) * 1000;
    
    ESP_LOGI("BATTERY VOLTAGE:", "Battery Voltage: %f", battery_voltage);

    return (int)battery_voltage;
}