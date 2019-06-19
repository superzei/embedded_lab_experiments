#include "UART.h"

#include "tm4c123ge6pm.h"

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

void Init_UART3(void){
	
    //Init UART for DFPlayer
    SysCtlPeripheralEnable(SD_UART_PERIPH);
    SysCtlPeripheralEnable(SD_UART_GPIO_PERIPH);

    GPIOPinConfigure(SD_UART_RX);
    GPIOPinConfigure(SD_UART_TX);
    GPIOPinTypeUART(SD_UART_GPIO_BASE, (SD_UART_RX_PIN | SD_UART_TX_PIN));

    UARTConfigSetExpClk(SD_UART_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));

    //UARTIntRegister(SD_UART_BASE, &UARTIntHandler);
    UARTIntEnable(SD_UART_BASE, (UART_INT_RX)); //only enable RX and TX interrupts
    IntEnable(SD_UART_BASE); //enable the UART interrupt
    IntMasterEnable(); //enable processor interrupts


}


void Init_UART0(void){
	
    //Init UART for DFPlayer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, (SD_UART_RX_PIN | SD_UART_TX_PIN));

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
		
		UARTEnable(UART0_BASE);
}

void UARTIntHandlerOLD(){

			
}

void UARTPrint(char* message)
{
	for(int i = 0; i < sizeof(message); i++)
	{
		UARTCharPut(UART0_BASE, message[i]);
	}
}
