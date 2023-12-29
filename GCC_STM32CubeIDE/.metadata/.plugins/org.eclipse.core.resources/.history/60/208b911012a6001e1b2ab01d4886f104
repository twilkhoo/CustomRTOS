#include "timebase.h"
#include "stm32f4xx.h"

#define ONE_SEC_LOAD 		16000000

#define CTRL_ENABLE			(1<<0)
#define CTRL_TICKINT		(1<<1)
#define CTRL_CLKSRC			(1<<2)
#define CTRL_COUNTFLAG		(1<<16)

#define MAX_DELAY 0xFFFFFFFF

volatile uint32_t g_curr_tick;
volatile uint32_t g_curr_tick_p;
volatile uint32_t tick_freq = 1;

void timebase_init()
{
	// Reload timer with number of cycles per second.
	SysTick->LOAD = ONE_SEC_LOAD - 1;

	// Clear SysTick current value reg.
	SysTick->VAL = 0;

	// Select internal clock source.
	SysTick->CTRL = CTRL_CLKSRC;

	// Enable interrupt.
	SysTick->CTRL |= CTRL_TICKINT;

	// Enable SysTick.
	SysTick->CTRL |= CTRL_ENABLE;

	// Enable global interrupts.
	__enable_irq();
}

uint32_t get_tick()
{
	__disable_irq();
	g_curr_tick_p = g_curr_tick;
	__enable_irq();
	return g_curr_tick_p;
}

// Delay in seconds.
void delay(uint32_t delay)
{
	uint32_t tickstart = get_tick();
	uint32_t wait = delay;
	if (wait < MAX_DELAY)
	{
		wait += tick_freq;
	}

	while ((get_tick() - tickstart) < wait);
}

void tick_increment()
{
	g_curr_tick += tick_freq;
}

void SysTick_Handler(void)
{
	tick_increment();
}





