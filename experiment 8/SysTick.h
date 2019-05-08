#ifndef SysTick_h
#define SysTick_h

void SystickInit(void);
void SysTick_Handler(void);
void delay(unsigned long m);
unsigned long millis(void);

#endif
