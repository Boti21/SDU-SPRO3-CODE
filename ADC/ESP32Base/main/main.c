#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define A4 ADC1_CHANNEL_4 // which analog is used, The channel depends on which GPO we want to use
//GPIO 32 => ADC 1, CHANNEL 4

// Functions
void setup_adc(void);


void app_main() {

   int val_pot = 0;
   
   setup_adc();

    while(1){
        // Set pin GPO32 as an input
        gpio_num_t pinNumber = GPIO_NUM_32;
        gpio_set_direction(pinNumber, GPIO_MODE_INPUT);


        // read ADC, print ADC
        val_pot = adc1_get_raw(A4); // It saves the readings of the A4
        printf("The value of the ADC: %d\n", val_pot);
        vTaskDelay(300/portTICK_PERIOD_MS); // Delay 300 ms

        // calculate Vout ==> Vout = Dout* Vmax/Dmax
    }
}

void setup_adc(void){

    // Configuration of ADC
    adc1_config_channel_atten(A4, ADC_ATTEN_DB_11); // (channel we want to put for attenuation, which attenuation do we want)
    // Attenuation of db 11 is the biggest one, it allows the biggest range of reading
    adc1_config_width(ADC_WIDTH_BIT_12); // The resolution of all the adc1; 12 bits

}









