#ifndef UART_h
#define UART_h

unsigned char UART_InChar(void);
void UART_OutChar(unsigned char data);
void flushRX(void);
void InitUART(void);
void enableInterrupts(void);
void disableInterrupts(void);

#endif
