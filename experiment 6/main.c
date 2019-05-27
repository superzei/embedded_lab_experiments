// main.c
// Runs on  TM4C123
// Baris Yildiz, Ali Batuhan Undar
// March 24, 2019
// LaunchPad built-in hardware

// OUT: PB1-2: 0x06: 0000 0110
// |
// |_ PB-1 (0x02): data (tx)
// |
// |_ PB-2 (0x04): clock (clk)

#include "tm4c123ge6pm.h"

/////////////////////////
//////// FLAGS //////////

const char DATA[] = "Hello World!";
const int length = sizeof(DATA) / sizeof(char);
const int bit_len = length * 8;
int char_index = 0;
int bit_index = 0x00;

/////////////////////////
/////////////////////////

void PortB_Init(void)
{
	
// PB5 for the switch Button
  SYSCTL_RCGC2_R |= 0x00000002;	// activate clock for Port B
  while ((SYSCTL_PRGPIO_R & 0x02) == 0) {};	//Wait until clock bit is setted.
  GPIO_PORTB_LOCK_R = 0x4C4F434B;	// unlock GPIO Port B
  GPIO_PORTB_CR_R |= 0x06;	// allow changes
  GPIO_PORTB_AMSEL_R &= ~0xFF;	// disable analog on PB
  GPIO_PORTB_PCTL_R = 0x00000000;	// PCTL GPIO on PB2-1
  GPIO_PORTB_DIR_R |= 0x06;	// PB2-1 out
  GPIO_PORTB_AFSEL_R = 0x00;	// disable alt funct on PB2-1
  GPIO_PORTB_DEN_R |= 0x06;	// enable digital I/O on PB2-1
	GPIO_PORTB_DATA_R &= ~0x06;	// clear bits
}

void SysTick_Init(int clocks_per_tick)
{
  NVIC_ST_CTRL_R = 0;	// disable SysTick during setup
	//NVIC_ST_RELOAD_R = animation_step_in_miliseconds * 16000; // seconds_to_50_miliseconds * clock_cycle_to_10_milisecond
	NVIC_ST_RELOAD_R = clocks_per_tick;	// interrupt on every "x" clock cycle
	NVIC_ST_CURRENT_R = 0;	// clear current
  NVIC_ST_CTRL_R = 0x00000007;	// enable SysTick with core clock and iterrupt
	NVIC_SYS_PRI3_R |= 0x40000000;	// set interrupt priority to "010"
}

void SysTick_Handler(void)
{
	GPIO_PORTB_DATA_R &= ~0x02;	// clear data bit
	int next_bit = (((DATA[char_index] & (bit_index)) && 0x1));	// if next bit is 1 set 1, else set 0
	
	if (GPIO_PORTB_DATA_R & 0x04)	// next is falling edge, set data
		GPIO_PORTB_DATA_R |= (0x02 * next_bit);		// set the next bit as data
	
	GPIO_PORTB_DATA_R ^= 0x04;	// tick clock 
	
	if (bit_index == 0x80)	// if bits going to reset on next tick,
		char_index = (char_index + 1) % length;	// increment the char index
	if (bit_index == 0x00)	// bit index is 0,
		bit_index += 0x01;	// increment bit index, can't shift 0
	else
		bit_index = ((bit_index << 1) & 0xFF);	// increment the bit index

	// TODO: null termination??
}


int main(void)
{
	SysTick_Init(1000);	// a systick per x clock cycles
	
	PortB_Init();

	
	while(1)
	{

	}
}
