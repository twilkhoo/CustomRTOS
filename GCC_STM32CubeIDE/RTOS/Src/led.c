#include "led.h"

#define GPIOAEN (1<<0) // GPIOA enable reg
#define LED_PIN (1<<5)

void led_init()
{
	// Enable clock access to led port (port A, PA5).
	RCC->AHB1ENR |= GPIOAEN;

	// Set LED pin as output pin.
	GPIOA->MODER |= (1<<10);
	GPIOA->MODER &= ~(1<<11);
}

void led_on()
{
	// Set LED pin HIGH (PA5 high).
	GPIOA->ODR |= LED_PIN;
}


void led_off()
{
	// Set LED pin LOW (PA5 low).
	GPIOA->ODR &= ~LED_PIN;
}
