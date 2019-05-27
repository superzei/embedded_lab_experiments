// main.c
// Runs on LM4F120 or TM4C123
// Flashes SOS when button pressed, otherwise flashes colors in order
// Ali Batuhan ÜNDAR, Baris YILDIZ
// January 18, 2016

// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad


#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define PF4                     (*((volatile unsigned long *)0x40025040))
#define PF3                     (*((volatile unsigned long *)0x40025020))
#define PF2                     (*((volatile unsigned long *)0x40025010))
#define PF1                     (*((volatile unsigned long *)0x40025008))
#define PF0                     (*((volatile unsigned long *)0x40025004))
#define GPIO_PORTF_DR2R_R       (*((volatile unsigned long *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile unsigned long *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile unsigned long *)0x40025508))
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
	
#define LED_RED									0x02
#define LED_BLUE								0x04
#define LED_GREEN								0x08
#define LED_WHITE								0x0E

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

int SOS_wanted = 0;
void read_sw4()
{
	// read if switch is pressed
	if (!(GPIO_PORTF_DATA_R & 0x10))
		SOS_wanted = 1;
}

void Delay(double const seconds){
	// wait for a specific amount of time
	unsigned long volatile time;
  time = 1454480 * seconds * 0.6;  // new if check adds 6 more cycles to previous 7 cycles, so, it takes a bit more than double now
  while(time){
		
		if(!(time % 8))		// check if the button pressed (polling)
			read_sw4();
		
		time--;
  }
	read_sw4();
}

void t_flash(double const duration, volatile unsigned long LED)
{
	// three flashes of desired LEDs
	int i = 0;
	for(i = 0; i < 3; i++)
	{
			GPIO_PORTF_DATA_R |= LED;
			Delay(duration);
			GPIO_PORTF_DATA_R &= !LED;
			Delay(0.25);
	}
}

void SOS()
{
	/*
	Flashes a SOS signal
	3 short red flashes for 0.25 seconds
	3 long white flashes for 0.5 seconds
	then 3 short flashes for 0.25 seconds
	all 0.25 seconds apart
	*/
	SOS_wanted = 0;
	t_flash(0.25, LED_RED);
	t_flash(0.5, LED_WHITE);
	t_flash(0.25, LED_RED);

}


int main(void){  
  PortF_Init();  // make PF1 out (PF1 built-in LED)
	GPIO_PORTF_DATA_R = 0x00;
  while(1){
		if (SOS_wanted)
		{
			SOS();
			continue;
		}
		
		GPIO_PORTF_DATA_R |= LED_RED;
		Delay(0.5);
		GPIO_PORTF_DATA_R &= !LED_RED;
		
		GPIO_PORTF_DATA_R |= LED_BLUE;
		Delay(1.0);
		GPIO_PORTF_DATA_R &= !LED_BLUE;
		
		GPIO_PORTF_DATA_R |= LED_GREEN;
		Delay(1.5);
		GPIO_PORTF_DATA_R &= !LED_GREEN;
  }
}
