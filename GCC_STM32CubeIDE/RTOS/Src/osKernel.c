#include "osKernel.h"

#define NUM_OF_THREADS		3
#define STACKSIZE			400

#define BUS_FREQ			16000000

#define CTRL_ENABLE			(1<<0)
#define CTRL_TICKINT		(1<<1)
#define CTRL_CLKSRC			(1<<2)
#define CTRL_COUNTFLAG		(1<<16)


uint32_t MILLIS_PRESCALER;

typedef struct tcb {
	uint32_t* stackPt;
	struct tcb* nextPt;
} tcbType;

tcbType tcbs[NUM_OF_THREADS];

tcbType *currentPt;

// Each thread has STACKSIZE 100 => 400 bytes (32 bit words).
uint32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

void osKernelStackInit(uint32_t i)
{

	// Set the current thread's stack pointer to the last location that we'll push stuff to.
	tcbs[i].stackPt = &TCB_STACK[i][STACKSIZE - 16];

	// Set bit 21 of the PSR to 1, for thumb mode.
	TCB_STACK[i][STACKSIZE - 1] = (1<<24);

	// R0...3, 12, LR (link register) are what an exception
	// pushes onto a stack automatically.
	// SP=>R13, LR=>R14, PC=>R15.

	// We don't need to init these regs explicitly, but we do so for debugging.
	TCB_STACK[i][STACKSIZE - 3] = 0xAAAAAAAA;		// LR.
	TCB_STACK[i][STACKSIZE - 4] = 0xAAAAAAAA;		// R12.
	TCB_STACK[i][STACKSIZE - 5] = 0xAAAAAAAA;		// R3.
	TCB_STACK[i][STACKSIZE - 6] = 0xAAAAAAAA;		// R2.
	TCB_STACK[i][STACKSIZE - 7] = 0xAAAAAAAA;		// R1.
	TCB_STACK[i][STACKSIZE - 8] = 0xAAAAAAAA;		// R0.

	TCB_STACK[i][STACKSIZE - 9] = 0xAAAAAAAA;		// R11.
	TCB_STACK[i][STACKSIZE - 10] = 0xAAAAAAAA;		// R10.
	TCB_STACK[i][STACKSIZE - 11] = 0xAAAAAAAA;		// R9.
	TCB_STACK[i][STACKSIZE - 12] = 0xAAAAAAAA;		// R8.
	TCB_STACK[i][STACKSIZE - 13] = 0xAAAAAAAA;		// R7.
	TCB_STACK[i][STACKSIZE - 14] = 0xAAAAAAAA;		// R6.
	TCB_STACK[i][STACKSIZE - 15] = 0xAAAAAAAA;		// R5.
	TCB_STACK[i][STACKSIZE - 16] = 0xAAAAAAAA;		// R4.
}


void osKernelInit()
{
	MILLIS_PRESCALER = (BUS_FREQ/1000); // 16000hz is 1ms.
}


// Define the tasks (functions) to run for the 3 threads explicitly as function ptrs,
// and setup the nextPt for each thread to enable round-robin.
uint8_t osKernelAddThreads(void(*task0)(), void(*task1)(), void(*task2)())
{
	// Disable global interrupts.
	__disable_irq();

	// Setup next threads for each thread, circularly.
	tcbs[0].nextPt = &tcbs[1];
	tcbs[1].nextPt = &tcbs[2];
	tcbs[2].nextPt = &tcbs[0];

	// Initial stack for thread 0.
	osKernelStackInit(0);
	// Initialize PC for thread 0.
	TCB_STACK[0][STACKSIZE - 2] = (uint32_t)task0;

	// Initial stack for thread 1.
	osKernelStackInit(1);
	// Initialize PC for thread 1.
	TCB_STACK[1][STACKSIZE - 2] = (uint32_t)task1;

	// Initial stack for thread 2.
	osKernelStackInit(2);
	// Initialize PC for thread 2.
	TCB_STACK[2][STACKSIZE - 2] = (uint32_t)task2;


	// Assign current thread to be 0th task.
	currentPt = &tcbs[0];

	// Enable global interrupts.
	__enable_irq();

	return 1;
}

// Launches the first thread (0), and never executes again.
void osSchedulerLaunch()
{
	// Load address of currentPt into R0.
	// R0 = &currentPt.
	__asm("LDR 		R0,=currentPt");

	// Load value at R0 (* (&currentPt)) into R2.
	// R2 = currentPt.
	__asm("LDR 		R2,[R0]");

	// Load the stack pointer from value at R2.
	// SP = *currentPt = currentPt->stackPt.
	__asm("LDR 		SP,[R2]");

	// Restore R4...R11.
	__asm("POP 		{R4-R11}");

	// Restore R12.
	__asm("POP 		{R12}");

	// Restore R0...R3.
	__asm("POP 		{R0-R3}");

	// Skip restoring LR for current thread.
	__asm("ADD		SP, SP, #4");

	// Create a new start location by popping current address into LR.
	// This represents the current address of the top of this thread's stack.
	__asm("POP 		{LR}");

	// Skip restoring PSR.
	__asm("ADD		SP, SP, #4");

	// Enable global interrupts.
	__asm("CPSIE 	I");

	// Return from exception (automatically restores R0...3, 12).
	__asm("BX 		LR");
}


void osKernelLaunch(uint32_t quanta)
{
	// Reset SysTick.
	SysTick->CTRL = 0;

	// Clear SysTick's current value reg.
	SysTick->VAL = 0;

	// Load quanta.
	SysTick->LOAD = (quanta * MILLIS_PRESCALER) - 1;

	// Set SysTick to low priority.
	NVIC_SetPriority(SysTick_IRQn, 15);

	// Enable SysTick.
	SysTick->CTRL = CTRL_ENABLE;

	// Select internal clock.
	SysTick->CTRL |= CTRL_CLKSRC;

	// Enable SysTick interrupts.
	SysTick->CTRL |= CTRL_TICKINT;

	// Launch scheduler.
	osSchedulerLaunch();
}

// __attribute__ is a GNU GCC hint, like a decorator.
// naked indicates to generate he following function without a prologue or epilogue,
// which means no setting up/tearing down stack frame, no saving/restoring registers, etc.
// When exception occurs, R0...3, 12 are automatically saved.
__attribute__((naked)) SysTick_Handler()
{
	// SUSPEND CURRENT THREAD.
	// Disable global interrupts.
	__asm("CPSID	I");

	// Save R4...R11, since these aren't automatically saved on stack.
	__asm("PUSH		{R4-R11}");

	// Load the address of currentPt into R0, which would basically be type tcbType**.
	// R0 = &currentPt.
	__asm("LDR		R0, =currentPt");

	// Load the actual memory at **currentPt, which would be *currentPt- which points to the actual current struct tcbType.
	// R1 = *R0 = currentPt.
	__asm("LDR 		R1,[R0]");

	// Store Cortex-M SP at address R1, which is the tcbType (so, we're setting this thread's stackPt).
	// *R1 = SP = stackPt.
	__asm("STR 		SP,[R1]");


	// CHOOSE NEXT THREAD.
	// Load R1 from the location 4 bytes above R1, since 4 bytes above R1 (stackPt), we have nextPt,
	// so loading from this location nextPt essentially loads the next thread into R1.
	// R1 = currentPt->nextPt.
	__asm("LDR 		R1, [R1, #4]");

	// Store R1 at the address R0.
	// *R0 = R1 = currentPt->nextPt;
	__asm("STR 		R1, [R0]");

	// Load SP from address R1.
	// SP = currentPt->nextPt.
	__asm("LDR 		SP,[R1]");

	// Pop R4...R11.
	__asm("POP 		{R4-R11}");

	// Enable global interrupts.
	__asm("CPSIE 	I");

	// Return from exception (automatically restores R0...3, 12).
	__asm("BX 		LR");
}

void osSemaphoreInit(int32_t* semaphore, int32_t value)
{
	*semaphore = value;
}

void osSemaphorePost(int32_t* semaphore)
{
	// Disable global interrupts.
	__disable_irq();

	*semaphore += 1;

	// Enable global interrupts.
	__enable_irq();
}


void osSemaphoreWait(int32_t* semaphore)
{
	// Disable global interrupts.
	__disable_irq();


	while (*semaphore <= 0) // Basically spinning.
	{
		__disable_irq();
		__enable_irq();
	}

	*semaphore -= 1;

	// Enable global interrupts.
	__enable_irq();
}




















