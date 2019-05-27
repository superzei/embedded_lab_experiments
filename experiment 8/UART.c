#include "UART.h"
#include "esp8266.h"
#include "tm4c123ge6pm.h"

/**
Initialize UART1 with:
 - 8bit parity
 - One stop
 - No parity
 - 115200 baud rate 
*/
void InitUART(void)
{
	// UART
  SYSCTL_RCGC1_R |= 0x00000002;  // activate UART1
  SYSCTL_RCGC2_R |= 0x00000004;  // activate port C
  UART1_CTL_R &= ~0x00000001;    // disable UART
  UART1_IBRD_R = 43;     // IBRD = int(80,000,000/(16*115,200)) = int(43.40278)
  UART1_FBRD_R = 26;     // FBRD = round(0.40278 * 64) = 26
  UART1_LCRH_R = 0x00000070;  // 8 bit, no parity bits, one stop, FIFOs
  UART1_CTL_R |= 0x00000001;     // enable UART
	
	// STATUS LED
	GPIO_PORTC_DIR_R |= 0x40;				// PC6 output
	GPIO_PORTC_DEN_R |= 0x40;				// Enable PC6 as digital
	GPIO_PORTC_AMSEL_R &= ~0x40;		// Disable analog on PC6
	
	// RX, TX ports
  GPIO_PORTC_AFSEL_R |= 0x30;    // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;      // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xF000FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;   // disable analog on PC5-4
	
	// INTERRUPTS
	UART1_IFLS_R &= ~0x3F;                            // Clear TX and RX interrupt FIFO level fields
  UART1_IFLS_R += UART_IFLS_RX1_8 ;                 // RX FIFO interrupt threshold >= 1/8th full
  UART1_IM_R  |= UART_IM_RXIM | UART_IM_RTIM;       // Enable interupt on RX and RX transmission end
	// enableInterrupts();
}

/*
enables uart rx interrupt
LIBERATED FROM Steven Prickett (steven.prickett@gmail.com)
*/
void enableInterrupts(void){
  NVIC_EN0_R = 1<<6; // interrupt 6
}

/*
disables uart rx interrupt
LIBERATED FROM Steven Prickett (steven.prickett@gmail.com)
*/
void disableInterrupts(void){
  NVIC_DIS0_R = 1<<6; // interrupt 6
}

unsigned char UART_InChar(void){
  while((UART1_FR_R&0x0010) != 0);      // wait until RXFE is not 0
  return((unsigned char)(UART1_DR_R&0xFF));
}

// Wait for buffer to be not full, then output
void UART_OutChar(unsigned char data){
  while((UART1_FR_R&0x0020) != 0);      // wait until TXFF is not 0
  UART1_DR_R = data;
}

/*
	read 8 bits to flush the RX
	I must be drunk when I wrote this
*/
void flushRX(void)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (UART1_FR_R & UART_FR_RXFE)	// already empty
			break;
		UART_InChar();
	}
}
