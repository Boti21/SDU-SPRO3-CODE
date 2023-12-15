#ifndef FL_DISPLAY_H
#define FL_DISPLAY_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "fonts.h"

void init_display(void)
{
	SSD1306_Init();
}

void display_weight(int weight_in_grams)
{
	char print[5] = {0};
	char weight[2] = {0};
	uint8_t counter = 0;

	weight_in_grams /= 100;
	while(weight_in_grams) {
		weight[counter] = (3 << 4) | (weight_in_grams % 10);
		weight_in_grams /= 10;
		counter++;
	}
	//ESP_LOGI("display_weight", "%c %c %c \n", weight[2], weight[1], weight[0]);

	print[0] = weight[1];
	print[1] = '.';
	print[2] = weight[0];
	print[3] = 'k';
	print[4] = 'g';

	//FontDef_t font = Font_16x26;

	SSD1306_Puts(print, &Font_16x26, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
}