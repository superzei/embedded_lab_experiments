#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

#define SEC 1000000
#define MS 1000
#define mS 1

extern volatile uint32_t microseconds;

void Init_Systick(void);
void sleep(long long int duration);
uint32_t SysTickGetMicrosecond(void);
 
#endif
