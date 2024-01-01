#ifndef __OS_KERNEL_H__
#define __OS_KERNEL_H__

#include "stm32f4xx.h"
#include <stdint.h>

void osKernelInit();
void osKernelLaunch(uint32_t quanta);
uint8_t osKernelAddThreads(void(*task0)(), void(*task1)(), void(*task2)());

void osSemaphoreInit(int32_t* semaphore, int32_t value);
void osSemaphorePost(int32_t* semaphore);
void osSemaphoreWait(int32_t* semaphore);

#endif // __OS_KERNEL_H__
