#ifndef FL_DISPLAY_H
#define FL_DISPLAY_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/*
#include "ssd1306.h"
#include "fonts.h"

#define WEIGHT_PRINT 7
#define VOLTAGE_PRINT 6



void init_display(void)
{
	SSD1306_Init();
}

void display_weight(int weight_in_grams)
{
	char print[WEIGHT_PRINT] = {0};
	char weight[3] = {0};
	uint8_t counter = 0;

	weight_in_grams /= 10;
	while(weight_in_grams) {
		weight[counter] = (3 << 4) | (weight_in_grams % 10);
		weight_in_grams /= 10;
		counter++;
	}
	//ESP_LOGI("display_weight", "%c %c %c \n", weight[2], weight[1], weight[0]);

	print[0] = weight[2];
	print[1] = '.';
	print[2] = weight[1];
	print[3] = weight[0];
	print[4] = ' ';
	print[5] = 'k';
	print[6] = 'g';

	SSD1306_UpdateScreen();

	SSD1306_GotoXY(0, 6);
	for(int i = 0; i < WEIGHT_PRINT; i++) {
		SSD1306_Putc(print[i], &Font_16x26, SSD1306_COLOR_WHITE);
	}
	SSD1306_UpdateScreen();
}

void display_voltage(int v_in_mvolts)
{
	char print[VOLTAGE_PRINT] = {0};
	char volts[3] = {0};
	uint8_t counter = 0;

	v_in_mvolts /= 10;
	while(v_in_mvolts) {
		volts[counter] = (3 << 4) | (v_in_mvolts % 10);
		v_in_mvolts /= 10;
		counter++;
	}
	//ESP_LOGI("display_weight", "%c %c %c \n", weight[2], weight[1], weight[0]);

	print[0] = volts[2];
	print[1] = '.';
	print[2] = volts[1];
	print[3] = volts[0];
	print[4] = ' ';
	print[5] = 'V';

	SSD1306_UpdateScreen();

	SSD1306_GotoXY(0, 36);
	for(int i = 0; i < VOLTAGE_PRINT; i++) {
		SSD1306_Putc(print[i], &Font_16x26, SSD1306_COLOR_WHITE);
	}
	SSD1306_UpdateScreen();
}
*/
