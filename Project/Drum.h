#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#ifndef DRUM_H_
#define DRUM_H_

typedef struct Drum Drum;

struct Drum
{
	uint8_t id;
	uint32_t distance;
	uint32_t prev_distance;
	uint8_t folder_name;
	uint8_t file_name;
	
	uint8_t trigger_pin;
	uint8_t echo_pin;
	uint32_t trigger_int_pin;
};


void Init_drum(void);
void registerDrum(Drum*);
void update_distances(void);
void get_distances(uint32_t*);
void createDrum(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);

#endif
