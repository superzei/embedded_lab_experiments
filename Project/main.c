// BBM 434 PROJECT
// Runs on  TM4C123
// Generates sound which changed frequency with a potentiometer.
// PB0, PB1, PB2, PB3 are used for dac.
// PE2 for adc input.
// Ali Batuhan Undar, Baris Yildiz
// Jun 06, 2019


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"

#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/adc.h"

#include "DFPlayer.h"
#include "UART.h"
#include "Drum.h"
#include "SysTick.h"

#define UDS_PERIPH SYSCTL_PERIPH_GPIOE
#define UDS_BASE GPIO_PORTE_BASE
#define UDS_TRIGGER GPIO_PIN_5
#define UDS_ECHO GPIO_PIN_4

void EnableInterrupts(void); // Enable interrupts
void WaitForInterrupt(void);
void UARTIntHandler(void);

char buffer[512];
int bindex = 0;

void put(char in)
{
	bindex = (bindex + 1) & 512;
	buffer[bindex] = in;
}


int main(void)
{
	// Init_UART6();
	DFP_Init();
	
	createDrum(0, 0x01, 0x01, GPIO_PIN_2, GPIO_PIN_3, GPIO_INT_PIN_3);
	createDrum(1, 0x01, 0x02, GPIO_PIN_3, GPIO_PIN_2, GPIO_INT_PIN_2);
	createDrum(2, 0x01, 0x03, GPIO_PIN_4, GPIO_PIN_1, GPIO_INT_PIN_1);
	createDrum(3, 0x01, 0x04, GPIO_PIN_5, GPIO_PIN_4, GPIO_INT_PIN_4);
	Init_drum();
	
	//Init_UART0();
	
	/*DFP_play_file_in_folder(0x1, 0x64);
	DFP_play();
	sleep(60 * SEC);
	DFP_pause();*/
	
  while (1)
  {
		update_distances();	
		
		/*uint32_t buffer[4];
		char message[50];
		get_distances(buffer);
		
		sprintf(message, "results: %d %d %d %d\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
		//UARTPrint(message);
		
		SysCtlDelay(600000);*/
  }
	
}
