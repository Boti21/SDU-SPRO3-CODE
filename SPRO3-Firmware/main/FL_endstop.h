#ifndef FL_ENDSTOP_H
#define FL_ENDSTOP_H
#endif

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

#define ENDSTOP_UP GPIO_NUM_15
#define ENDSTOP_DOWN GPIO_NUM_14
/*
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
*/