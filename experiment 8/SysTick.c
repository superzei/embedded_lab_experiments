#include "SysTick.h"
#include "tm4c123ge6pm.h"
#include "PWM.h"

unsigned long milliseconds;

/*
	One interrupt a millisecond, keeps 
*/
void SystickInit(void)
{
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	//NVIC_ST_RELOAD_R = animation_step_in_miliseconds * 16000; // seconds_to_50_miliseconds * clock_cycle_to_10_milisecond
	NVIC_ST_RELOAD_R = 80000; // interrupt on every "x" clock cycle
	NVIC_ST_CURRENT_R = 0;				// clear current
	NVIC_SYS_PRI3_R |= 0x40000000;
	NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and iterrupt
	milliseconds = 0;
}


void SysTick_Handler(void)
{
	// ERROR, systick priority problems
	milliseconds++;	//keep rolling, rolling, rolling, HEY!.
}

/*
	Wait for "m" milliseconds
*/
void delay(unsigned long m)
{
	unsigned long current = milliseconds;
	while (milliseconds <= (current + m));
}

unsigned long millis(void)
{
	return milliseconds;
}

