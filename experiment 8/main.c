// main.c
// Runs on  TM4C123
// Baris Yildiz, Ali Batuhan Undar
// March 24, 2019
// LaunchPad built-in hardware

#include "tm4c123ge6pm.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "UART.h"
#include "esp8266.h"
#include "SysTick.h"


extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);

int PWM_freq = 16000;

void PWM_Init(void)
{
    /* Enable Peripheral Clocks */
    SYSCTL_RCGCPWM_R |= 0x2;       /* enable clock to PWM1 */
    SYSCTL_RCGCGPIO_R |= 0x20;   /* enable clock to PORTF */
    SYSCTL_RCC_R &= ~0x00100000; /* no pre-divide for PWM clock */

 
    /* Enable port PF3 for PWM1 M1PWM7 */
    GPIO_PORTF_AFSEL_R = 0x08;      /* PF3 uses alternate function */
    GPIO_PORTF_PCTL_R &= ~0x0000F000; /* make PF3 PWM output pin */
    GPIO_PORTF_PCTL_R |= 0x00005000;
    GPIO_PORTF_DEN_R |= 0x08;       /* pin digital */


    PWM1_3_CTL_R = 0x0;            /* stop counter */
    PWM1_3_GENB_R = 0x0000008C;  /* M1PWM7 output set when reload, */

    /* clear when match PWMCMPA */
    PWM1_3_LOAD_R = 16000;       /* set load value for 1kHz (16MHz/16000) */
    PWM1_3_CMPA_R = 15999;       /* set duty cycle to min */
    PWM1_3_CTL_R = 0x1;            /* start timer */
    PWM1_ENABLE_R = 0x80;        /* start PWM1 ch7 */
}

void PWM_Loop(void)
{
	int x = 16000;
	int inc = 100;
	
	for(;;)
	{
		x -= (inc);
		if (x <= 0 || x >= 16000){
			inc *= -1;
			continue;
		}
		PWM1_3_CMPA_R = x;
			
		delay(2);
	}

}

void set(int val)
{
	int old_val = PWM1_3_CMPA_R;
	if (old_val == val)
		return;
	
	int direction = ((val - old_val) > 0);
	int inc = -100;
	
	if (direction)
		inc = 100;

	while(1)
	{
		if ((old_val <= val && !direction) || (old_val >= val && direction))
			return;
		
		old_val += inc;
		PWM1_3_CMPA_R = old_val;
		
		delay(10);
	}

}

/**
	SET CLOCK RATE TO 80Mhz
**/
void PLL_Init(void)
{
	SYSCTL_RCC2_R |= 0x80000000; // USERCC2
	SYSCTL_RCC2_R |= 0x00000800; // BYPASS2, PLL bypass
	SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) // clear XTAL field, bits 10-6
				   + 0x00000540; // 10101, configure for 16 MHz crystal
	SYSCTL_RCC2_R &= ~0x00000070; // configure for main oscillator source
	SYSCTL_RCC2_R &= ~0x00002000; // activate PLL by clearing PWRDN
	SYSCTL_RCC2_R |= 0x40000000; // use 400 MHz PLL
	SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) // clear system clock divider
					+ (4 << 22); // configure for 80 MHz clock
	while ((SYSCTL_RIS_R & 0x00000040) == 0){};		// wait for the PLL to lock by polling PLLLRIS
	SYSCTL_RCC2_R &= ~0x00000800; // enable use of PLL by clearing BYPASS
}

void init()
{
	PLL_Init();
	InitUART();
	SystickInit();
	PWM_Init();
}


int main()
{
	init();
	EnableInterrupts();

	char IP[46] = "192.168.4.2";
	int port = 8080;
	char txbuffer[50];

	ATcommand("AT+RST", 1000, "OK"); // reset
	ATcommand("AT+CWQAP", 1000, "OK"); // disconnect
	ATcommand("AT+CWMODE=3", 1000, "OK");	// set mode to station + soft ap
	// ATcommand("AT+CWJAP=\"HUBBM\",\"donaldknuth\"", 10000, "OK");	// connect
	
	sprintf(txbuffer, "AT+CWSAP=\"%s\",\"%s\",%d,%d", "honorlessman", "1234567890", 5, 3);
	ATcommand(txbuffer, 2000, "OK");
	ATcommand("AT+CIPMUX=0", 500, "OK");	// set MULTI connection mode
	ATcommand("AT+CIFSR", 500, "OK");	// get server IP

	// PWM_Loop();
	/*int index = 0;
	int arr[5] = {1000, 5000, 4000, 8000, 16000};
	while(1)
	{
		set(arr[index]);
		index++;
		if (index >= 5)
			index = 0;
		delay(30);
	}*/
	
	int i_value;
	char *raw_val;
	// SERVER LOOP
	while(1)
	{	
		if (send_get_request((char*)IP, port))
		{
			 int start_index = SearchIndexOf(received, "data=");
				if (start_index != -1) // received response
				{
					char* start_point = received + start_index + 5;
					i_value = strtol(start_point, NULL, 10);
					// (i_value);
					PWM1_3_CMPA_R = i_value;
				}
			}
		}
		delay(50);
	
	// WaitForInterrupt();	// AHAHAHAHAHAHAHAHAHAHAHAHAHAHA
}
