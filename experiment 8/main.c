// main.c
// Runs on  TM4C123
// Baris Yildiz, Ali Batuhan Undar
// March 24, 2019
// LaunchPad built-in hardware

#include "tm4c123ge6pm.h" 
#include <stdio.h>
#include <string.h>

extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);

/****************VARS********************/

const unsigned int timeout = 10000; // timeout for read operations
static char received[512];
unsigned long milliseconds = 0;

/****************************************/

int SearchIndexOf(char src[], char str[]);
void readOutput(char* searchFor, unsigned long tm);

/**
Initialize UART1 with:
 - 8bit parity
 - One stop
 - No parity
 - 115200 baud rate 
*/
void InitUART()
{
  SYSCTL_RCGC1_R |= 0x00000002;  // activate UART1
  SYSCTL_RCGC2_R |= 0x00000004;  // activate port C
  UART1_CTL_R &= ~0x00000001;    // disable UART
  UART1_IBRD_R = 43;     // IBRD = int(80,000,000/(16*115,200)) = int(43.40278)
  UART1_FBRD_R = 26;     // FBRD = round(0.40278 * 64) = 26
  UART1_LCRH_R = 0x00000070;  // 8 bit, no parity bits, one stop, FIFOs
  UART1_CTL_R |= 0x00000001;     // enable UART
	
  GPIO_PORTC_AFSEL_R |= 0x30;    // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;      // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;   // disable analog on PC5-4
}

/*
	One interrupt a millisecond, keeps 
*/
void SystickInit()
{
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	//NVIC_ST_RELOAD_R = animation_step_in_miliseconds * 16000; // seconds_to_50_miliseconds * clock_cycle_to_10_milisecond
	NVIC_ST_RELOAD_R = 80000; // interrupt on every "x" clock cycle
	NVIC_ST_CURRENT_R = 0;				// clear current
	NVIC_SYS_PRI3_R |= 0x40000000;
	NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and iterrupt
}

/*
	Wait for "m" milliseconds
*/
void delay(unsigned long m)
{
	unsigned long current = milliseconds;
	while (milliseconds <= (current + m));
}

/**
	SET CLOCK RATE TO 80Mhz
**/
void PLL_Init(void)
{
	// 0) Use RCC2
	SYSCTL_RCC2_R |= 0x80000000; // USERCC2
	
	// 1) bypass PLL while initializing
	SYSCTL_RCC2_R |= 0x00000800; // BYPASS2, PLL bypass
	
	// 2) select the crystal value and oscillator source
	SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) // clear XTAL field, bits 10-6
				   + 0x00000540; // 10101, configure for 16 MHz crystal
	
	SYSCTL_RCC2_R &= ~0x00000070; // configure for main oscillator source
	
	// 3) activate PLL by clearing PWRDN
	SYSCTL_RCC2_R &= ~0x00002000;
	
	// 4) set the desired system divider
	SYSCTL_RCC2_R |= 0x40000000; // use 400 MHz PLL
	
	SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) // clear system clock divider
					+ (4 << 22); // configure for 80 MHz clock
	
	// 5) wait for the PLL to lock by polling PLLLRIS
	while ((SYSCTL_RIS_R & 0x00000040) == 0)
	{}; // wait for PLLRIS bit
		
	// 6) enable use of PLL by clearing BYPASS
	SYSCTL_RCC2_R &= ~0x00000800;
}

void init()
{
	PLL_Init();
	InitUART();
	SystickInit();
}

void SysTick_Handler(void)
{
	milliseconds++;	//keep rolling, rolling, rolling, HEY!.
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
*/
void flushRX()
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (UART1_FR_R & UART_FR_RXFE)	// already empty
			break;
		UART_InChar();
	}
}

/*
	Send an AT command via UART
	Idea shamelessly stolen from online example
*/
void ATcommand(char* command)
{
	// memset(received, 0, sizeof(received));	// clear the buffer
	while (*command != '\0')	// We do not want to end string with 0, we want to end it with newline
	{
		UART_OutChar(*command++);	// Write a char, increment the pointer
	}
	UART_OutChar('\r'); // Carriage return
	UART_OutChar('\n'); // Run command

	readOutput("OK", timeout);
	delay(500);
}


void readOutput(char* searchFor, unsigned long tm)
{
	int i = 0;
	char a;
	memset(received, 0, sizeof(received));	// clear the buffer
	unsigned long current = milliseconds;
	while (milliseconds - current < tm)	// keep reading, until timeout
	{
		while(!(UART1_FR_R&UART_FR_RXFE))	// while we have chars to read
			{
				a = UART_InChar();	// read a char from UART1
				if(a == '\0') continue;		
				received[i]= a;
				i++;
			}
		if (SearchIndexOf(received, "OK") != -1)	//When OK reply sent, do not wait for timeout, get out
		{
			break;
		}
		
	}
}

/*
	Substring search
	Shamelessly stolen
*/
int SearchIndexOf(char src[], char str[])
{
   int i, j, firstOcc;
   i = 0, j = 0;

   while (src[i] != '\0')
   {

      while (src[i] != str[0] && src[i] != '\0')
         i++;

      if (src[i] == '\0')
         return (-1); // nope, string ended

      firstOcc = i;

      while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0')
      {
         i++;
         j++;
      }

      if (str[j] == '\0')
         return (firstOcc);	// got it
      if (src[i] == '\0')
         return (-1); // string ended, sadly

      i = firstOcc + 1;
      j = 0;
   }

   return (-1);	// couldn't found it
}

void respond()
{
	char response[1000] = "<html> <head> </head> <h1>Merhaba Dunya</h1> </html>";	// MAX 2048 bytes can be send
	char command[30];

	
	snprintf(command, 30, "AT+CIPSEND=0,%d", strlen(response) * sizeof(char));	// we are sending data, catch
	ATcommand(command);
	if (SearchIndexOf(received, "OK") != -1)	// received signal
	{
		ATcommand(response);	// send the data
	}
	
	ATcommand("AT+CIPCLOSE=0");	// close connection
	
}

int main()
{
	init();
	EnableInterrupts();
	
	ATcommand("AT+RST"); // reset
	ATcommand("AT+CWQAP"); // disconnect
	ATcommand("AT+CWMODE=3");
	ATcommand("AT+CIPSTATUS");
	ATcommand("AT+CWJAP=\"TurkTelekom_TA65B\",\"NavaxmGc\"");	// connect
	ATcommand("AT+CIPMUX=1");	// set multi connection mode
	ATcommand("AT+CIPSERVER=1,80");	// start webserver at port 80
	
	/* SUPERLOOP */
	while(1)
	{
		readOutput("OK", 2000);
		if (SearchIndexOf(received, "+IPD") != -1) // if received data is a TCP message
		{
			respond();
		}
	}
	
	//ATcommand("AT+CWQAP"); // disconnect
	//readOutput();
	
	// WaitForInterrupt();
}
