#include "SysTick.h"

uint32_t current;
volatile uint32_t microseconds = 0; 

void SystickHandler(void);

void sleep(long long int duration)
{
	current = microseconds;
	while(microseconds - current < (duration / 10));
}


void Init_Systick(void)
{
	SysTickPeriodSet(0xa0); // every microsecond
	HWREG(NVIC_ST_CURRENT) = 1;
	SysTickIntEnable();
	
	SysTickIntRegister(SystickHandler);
	
	IntPrioritySet(FAULT_SYSTICK, 2);
	
	SysTickIntEnable();
	
	SysTickEnable();
	
}

void SystickHandler(void)
{
	microseconds++;
}

uint32_t SysTickGetMicrosecond(void)
{
	return microseconds;
}

