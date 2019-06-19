#include "Drum.h"
#include "SysTick.h"
#include "DFPlayer.h"


//#define DEBUG

#define DRUM_ECHO_PERIPH SYSCTL_PERIPH_GPIOE
#define DRUM_ECHO_BASE GPIO_PORTE_BASE

#define DRUM_TRIGGER_BASE GPIO_PORTA_BASE
#define DRUM_TRIGGER_PERIPH SYSCTL_PERIPH_GPIOA

// local variables
static volatile Drum *drums[4];
uint8_t drum_count;
uint32_t readings[4];

static volatile int waitforinterrupt = 1;

static volatile int current_drum = 0;
static volatile uint32_t trigger_time = 0;


// prototypes
void Init_Triggers(void);
void Init_Echoes(void);
void GPIOPortEHandler(void);

void Init_drum()
{
	//SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	Init_Systick();
	Init_Triggers();
	Init_Echoes();
	DFP_Init();
	IntMasterEnable();
}

void Init_Echoes(void)
{
	SysCtlPeripheralEnable(DRUM_ECHO_PERIPH);
	while(!SysCtlPeripheralReady(DRUM_ECHO_PERIPH));
	
	uint8_t pins = 0x00;
	uint32_t interrupts = 0x000000;
	for (int i = 0; i < drum_count; i++)
	{
		pins |= drums[i]->echo_pin;
		interrupts |= drums[i]->trigger_int_pin;
	}
		
	GPIOPinTypeGPIOInput(DRUM_ECHO_BASE, pins);
	GPIOIntTypeSet(DRUM_ECHO_BASE, pins, GPIO_BOTH_EDGES);
	GPIOIntRegister(DRUM_ECHO_BASE, &GPIOPortEHandler);
	
	IntPrioritySet(INT_GPIOE, 1);
		
	GPIOIntEnable(DRUM_ECHO_BASE, interrupts);
	IntEnable(INT_GPIOE);

}

void Init_Triggers(void)
{
	SysCtlPeripheralEnable(DRUM_TRIGGER_PERIPH);
	while(!SysCtlPeripheralReady(DRUM_TRIGGER_PERIPH));
	
	uint8_t pins = 0x00;
	for (int i = 0; i < drum_count; i++)
	{
		pins |= drums[i]->trigger_pin;
	}
		
	GPIOPinTypeGPIOOutput(DRUM_TRIGGER_BASE, pins);
}


void GPIOPortEHandler(void)
{
	GPIOIntClear(DRUM_ECHO_BASE, GPIO_INT_PIN_3 | GPIO_INT_PIN_2 | GPIO_INT_PIN_1 | GPIO_INT_PIN_4);
	volatile Drum *cd = drums[current_drum];
	int32_t status = GPIOPinRead(DRUM_ECHO_BASE, drums[current_drum]->echo_pin);
	
	if ((status & cd->echo_pin) == cd->echo_pin)
	{
		trigger_time = microseconds * 10;
	}
	else
	{
		uint32_t echo_time = microseconds * 10;
		uint32_t distance = (echo_time - trigger_time) / 58; // in cms
		
		cd->prev_distance = cd->distance;
		cd->distance = distance;
		
		// put drum handlers here

		if (distance <= 40 && distance >= 10)
		{
			DFP_play_file_in_folder(cd->folder_name, cd->file_name);
			SysCtlDelay(2000000); // about a sec
		}
		
		// drum handler end
		
		waitforinterrupt = 0;
	}
}

void registerDrum(Drum *new_drum)
{
	if (drum_count >= 4)
	{
		return;
	}
	
	drums[drum_count] = new_drum;
	drum_count++;
}

void createDrum
	(
		uint8_t id,
		uint8_t folder_name,
		uint8_t file_name,
		
		uint8_t trigger_pin,
		uint8_t echo_pin,
		uint32_t trigger_int_pin
	)
{
	Drum *drum = malloc(sizeof(Drum));
	drum->id = id;
	drum->folder_name = folder_name;
	drum->file_name = file_name;
	drum->trigger_pin = trigger_pin;
	drum->echo_pin = echo_pin;
	drum->trigger_int_pin = trigger_int_pin;
	
	drum->distance = 0;
	drum->prev_distance = 0;
	
	registerDrum(drum);
	// sweet memory leaks...
}

void update_distances(void)
{
	for (int i = 0; i < drum_count; i++)
	{
		waitforinterrupt = 1;
		current_drum = i;
		
		GPIOPinWrite(DRUM_TRIGGER_BASE, drums[current_drum]->trigger_pin, drums[current_drum]->trigger_pin);
		sleep(200 * mS);
		GPIOPinWrite(DRUM_TRIGGER_BASE, drums[current_drum]->trigger_pin, ~drums[current_drum]->trigger_pin);
		
		while(waitforinterrupt);
	}
}

void get_distances(uint32_t *buf)
{
	for (int i = 0; i < drum_count; i++)
	{
		buf[i] = drums[i]->distance;
	}
	
}
