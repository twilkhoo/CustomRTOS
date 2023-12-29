#ifndef __TIMEBASE_H__
#define __TIMEBASE_H__

#include <stdint.h>

void timebase_init();
uint32_t get_tick();
void delay(uint32_t delay);


#endif // __TIMEBASE_H__
