// main.c
// Runs on  TM4C123
// C2_Toggle_PF1, toggles PF1 (red LED) at 5 Hz
// Baris Yildiz, Ali Batuhan Undar
// March 24, 2019
// PB2 Green LED, PB3 Yellow LED and PB7 for Red LED
// PB5 for the switch Button
// LaunchPad built-in hardware

#include "tm4c123ge6pm.h"

//PORTB: 1010 1100 = 0xAC
//PORTF: 0000 0001 = 0x01

/////////////////////////
//////// FLAGS //////////

int systick_interrupt = 0;
int main_anim_reverse = 0;

int extra = 0;
int extra_anim_reverse = 0;
int extra_reversed = 0;

volatile double seconds = 0.0;

// debouncers
int locked = 0;
int extra_locked = 0; // when simple locks just wasn't enough, jk...

/////////////////////////
/////////////////////////


/////////////////////////
/////// ANIMATION ///////

unsigned long order[] = {
	0x00000004, // PB2 for Green LED
	0x00000008, // PB3 for Yellow LED
	0x00000080  // PB7 for Red LED
};

unsigned long reverse_order[] = {
	0x00000080, // PB7 for Red LED
	0x00000008, // PB3 for Yellow LED
	0x00000004, // PB2 for Green LED

};

unsigned long *main_animations[3] = {order, reverse_order};
int current = 0;

unsigned long extra_animation[] = 
{
	0x00000000,
	0x00000080,
	0x00000008,
	0x00000004,
	0x00000084,
	0x0000000C,
	0x0000008C,
	
	0x0000008c,
	0x00000088,
	0x00000084,
	0x00000080,
	0x00000008,
	0x00000004,
	0x00000000
};

int main_animation_steps = 3;
int extra_animation_steps = 14;

int main_animation_index = 0;
int extra_animation_index = 0;

int extra_next_step() {
	if (extra_reversed)
		return (extra_animation_steps - extra_animation_index);
	else
		return (extra_animation_index);
}


/////////////////////////
/////////////////////////

void PortF_Init(void)
{
  SYSCTL_RCGC2_R |= 0x00000020;     // activate clock for Port F
  while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}; //Wait until clock bit is setted.
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x11;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R &= ~0xFF;        // disable analog on PF
  GPIO_PORTF_PCTL_R &= 0x00000000;   // PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R &= ~0x11;          // PF4,PF0 in
  GPIO_PORTF_AFSEL_R &= ~0xFF;        // disable alt funct on PF7-0
  GPIO_PORTF_PUR_R |= 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= 0x11;          // enable digital I/O on PF4/0
		
	// INTERRUPT INIT
	GPIO_PORTF_IS_R &= ~0x11;     // PF4,0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    // PF4,0 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    	// PF4,0 rising edge event
  GPIO_PORTF_ICR_R = 0x11;      // clear flag4,0
  GPIO_PORTF_IM_R |= 0x11;      // arm interrupt on PF4,0
	NVIC_PRI7_R |= 0x00200000;
  NVIC_EN0_R |= 0x40000000; 			// enable interrupt on PORTF
	
}

void PortB_Init(void)
{
	//PB2 Green LED, PB3 Yellow LED and PB7 for Red LED
// PB5 for the switch Button
  SYSCTL_RCGC2_R |= 0x00000002;     // activate clock for Port B
  while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}; //Wait until clock bit is setted.
  GPIO_PORTB_LOCK_R = 0x4C4F434B;   // unlock GPIO Port B
  GPIO_PORTB_CR_R |= 0xAC;           // allow changes
  GPIO_PORTB_AMSEL_R &= ~0xFF;        // disable analog on PB
  GPIO_PORTB_PCTL_R = 0x00000000;   // PCTL GPIO on PB4-0
  GPIO_PORTB_DIR_R |= 0x8C;          // PB2,3,7 out,PB5 in
  GPIO_PORTB_AFSEL_R = 0x00;        // disable alt funct on PB7-0
  GPIO_PORTB_DEN_R |= 0xAC;          // enable digital I/O on PB4/0
	GPIO_PORTB_DATA_R |= 0x8C; //Turn off all leds
		
	// INTERRUPT INIT
	GPIO_PORTB_IS_R &= ~0x20;     // PB5 is edge-sensitive
  GPIO_PORTB_IBE_R &= ~0x20;    // PB5 is not both edges
  GPIO_PORTB_IEV_R &= ~0x20;    	// PB5 falling edge event
  GPIO_PORTB_ICR_R = 0x20;      // clear flag5
  GPIO_PORTB_IM_R |= 0x20;      // arm interrupt on PB5
	NVIC_PRI0_R |= 0x0002000;
  NVIC_EN0_R |= 0x00000002; 			// enable interrupt on PORTB
}

void SysTick_Init(double animation_step_in_miliseconds)
{
  NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = animation_step_in_miliseconds * 16000; // seconds_to_50_miliseconds * clock_cycle_to_10_milisecond
	NVIC_ST_CURRENT_R = 0;	// clear current
  NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and iterrupt
	NVIC_SYS_PRI3_R |= 0x80000000;	// set interrupt priority to "100"
}

void SysTick_Handler(void)
{
	// each systick interrupts step up the animation
	main_animation_index = (main_animation_index + 1) % main_animation_steps;
	extra_animation_index = (extra_animation_index + 1) % extra_animation_steps;
}
void GPIOPortB_Handler(void)
{
	// we got the flag, ty
	GPIO_PORTB_ICR_R = 0x20;
	
	if (extra)
	{
		main_animation_index = 0;
		extra = 0;
	}
	else
	{
		main_anim_reverse = 1;
	}
}

void GPIOPortF_Handler(void)
{
	
	// we saw the flag
	GPIO_PORTF_ICR_R = 0x11;
	
	if (extra)
	{		
		// set flag for reversing animation
		extra_anim_reverse = 1;
	}
	else
	{
		// if playing main animation, switch over to extra animation
		extra = 1;
		extra_animation_index = 0;
	}
		
}

void main_anim()
{
	// die Hauptanimation (simple left-right or right-left animation)
	
	// switch animation direction
	if (main_anim_reverse && main_animation_index == 0){
		current = (current + 1) % 2;
		main_anim_reverse = 0;
		NVIC_ST_CURRENT_R = 0x0;
	}
	
	GPIO_PORTB_DATA_R |= 0x8C;
	GPIO_PORTB_DATA_R = ~main_animations[current][main_animation_index];

}

void extra_anim()
{
	// extra animation where LEDs collect into line

	if (extra_anim_reverse && extra_animation_index == 0){
		extra_reversed = (extra_reversed + 1) % 2;
		extra_anim_reverse = 0;
		NVIC_ST_CURRENT_R = 0x0;
	}
	
	GPIO_PORTB_DATA_R |= 0x8C;
	GPIO_PORTB_DATA_R = ~extra_animation[extra_next_step()];

}

int main(void)
{
	SysTick_Init(250);
	
	PortB_Init();
	PortF_Init();

	
	while(1)
	{
		if(extra)
		{
			extra_anim();
		}
		else
		{
			main_anim();
		}
	}
}
