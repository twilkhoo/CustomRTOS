/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "led.h"
#include "uart.h"
#include <stdio.h>

int main(void)
{
	led_init();
	uart_tx_init();

	while(1)
	{
		led_on();
		for(int i = 0; i < 900000; i++);
		led_off();
		for(int i = 0; i < 900000; i++);
		printf("Hello from STM32F411RE.......\n\r");
	}
}
