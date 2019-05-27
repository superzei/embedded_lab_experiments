// main.c
// Experiment 5 BBM 434. Runs on  TM4C123.
// C2_Toggle_PF1, toggles PF1 (red LED) at 5 Hz
// Baris Yildiz, Ali Batuhan Undar
// March 24, 2019
// PB2 Green LED, PB3 Red LED
// PB4 for Green Switch Button, PB5 for Red Switch Button
// LaunchPad built-in hardware

#include "tm4c123ge6pm.h"

#include "Nokia5110.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//PORTB: 0001 1110 = 1A for cr
//PORTB: 0000 0110 = 06 for dir

enum GameState {
  INITIALIZE = 1, PRESS_SCREEN_WAITING = 2, BUTTON_WAITING = 3, END_SCREEN = 4
};

enum GameState currentState;

int systick_interrupt = 0;

int isScoreShowing = 0;

//Constant
const unsigned int gameStartDelay = 3000; //in ms
const unsigned int scoreShowTime = 5000; //in ms

//Timers
unsigned long milliseconds = 0;
unsigned int gameStartScreenStartTime = 0;

unsigned int gameSessionStartTime = 0;
unsigned int gameSessionEndTime = 0;

unsigned int nextGameSessionStartTime = 0;
unsigned int scoreDisplayEndTime = 0;

unsigned int buttonWaitingStartTime = 0;

unsigned int greenPlayerPressedTime = 0;
unsigned int redPlayerPressedTime = 0;

time_t t; // For random generator

// Players
unsigned int greenPreesedTime;
unsigned int redPressedTime;

void PortB_Init(void) {
  // PB2 Green LED, PB3 Red LED 
  // PB4 for Green Switch Button, PB5 for Red Switch Button
  SYSCTL_RCGC2_R |= 0x00000002; // activate clock for Port B
  while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}; //Wait until clock bit is setted.
  GPIO_PORTB_LOCK_R = 0x4C4F434B; // unlock GPIO Port B
  GPIO_PORTB_CR_R |= 0x1A; // allow changes
  GPIO_PORTB_AMSEL_R &= ~0xFF; // disable analog on PB
  GPIO_PORTB_PCTL_R = 0x00000000; // PCTL GPIO on PB4-0
  GPIO_PORTB_DIR_R |= 0x06; // PB2, PB3 out, PB4 and PB5 in
  GPIO_PORTB_AFSEL_R = 0x00; // disable alt funct on PB7-0
  GPIO_PORTB_DEN_R |= 0x1A; // enable digital I/O on PB2/5
  GPIO_PORTB_DATA_R |= 0xF6; //Turn off all leds 

  // INTERRUPT INIT for rising edge
  GPIO_PORTB_IS_R &= ~0x18; // PB4 and PB5 is edge-sensitive
  GPIO_PORTB_IBE_R &= ~0x18; // PB4 and PB5 is not both edges
  GPIO_PORTB_IEV_R |= 0x18; // PB4 abd PB5 rising edge event
  GPIO_PORTB_ICR_R &= ~0x18; // clear flag PB4 and PB5
  GPIO_PORTB_IM_R |= 0x18; // arm interrupt on PB4 and PB5
  NVIC_PRI0_R |= 0x0002000;
  NVIC_EN0_R |= 0x00000002; // enable interrupt on PORTB
}

void SysTick_Init(unsigned long milisecond) {
  NVIC_ST_CTRL_R = 0; // disable SysTick during setup
  NVIC_ST_RELOAD_R = milisecond * 16000 - 1; // seconds_to_50_miliseconds * clock_cycle_to_10_milisecond
  NVIC_ST_CURRENT_R = 0; // clear current
  NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and iterrupt
  NVIC_SYS_PRI3_R |= 0x80000000; // set interrupt priority to "100"
}

void SysTick_Handler(void) {
  milliseconds++;
}

void GPIOPortB_Handler(void) {
  // we got the flag, ty
  GPIO_PORTB_ICR_R = 0x20;

  if (currentState == BUTTON_WAITING) {
    if ((GPIO_PORTB_DATA_R & 0x08) == 0x08 && greenPlayerPressedTime == 0) { //Green pressed
			greenPlayerPressedTime = milliseconds;
    } 
		
		if ((GPIO_PORTB_DATA_R & 0x10) == 0x10 && redPlayerPressedTime == 0) { //Red pressed
			redPlayerPressedTime = milliseconds;
    }
  }
}

void SysTick_Wait(unsigned long delay) {

  NVIC_ST_RELOAD_R = delay - 1; // number of counts to wait

  NVIC_ST_CURRENT_R = 0; // any value written to CURRENT clears

  while ((NVIC_ST_CTRL_R & 0x00010000) == 0) { // wait for count flag

  }
}

void wait(unsigned long seconds) {
  unsigned long delay = 100 * seconds;
  unsigned long i;

  for (i = 0; i < delay; i++) {
    // 1600000*6.25ns equals 10ms
    SysTick_Wait(160000); // wait 10ms
  }
}

void writeToScreen(char * str, int x, int y, int clear) {
	if(clear)
		Nokia5110_Clear();
  Nokia5110_SetCursor(x, y);
  Nokia5110_OutString(str);
	//Nokia5110_DisplayBuffer(); // draw buffer
}

int main(void) {
  PortB_Init();
	SysTick_Init(1);
	EnableInterrupts();
  Nokia5110_Init();
  Nokia5110_ClearBuffer();
  Nokia5110_DisplayBuffer(); // draw buffer

  currentState = INITIALIZE;
  /* Intializes random number generator */
  srand((unsigned) time(&t));
	
	isScoreShowing = 0;
	
  while (1) {
    if (!isScoreShowing) {
			
      if (currentState == INITIALIZE) {
        GPIO_PORTB_DATA_R |= 0xF6; //Turn off all leds 
				
				greenPlayerPressedTime = 0;
				redPlayerPressedTime = 0;
				
        writeToScreen("GET READY", 1, 1, 1);

        nextGameSessionStartTime = milliseconds + gameStartDelay;

        currentState = PRESS_SCREEN_WAITING;
      }

      if (currentState == PRESS_SCREEN_WAITING && milliseconds >= nextGameSessionStartTime) {
        int randomWaitSecond = rand() % 10;
        gameSessionStartTime = milliseconds;
        gameSessionEndTime = milliseconds + randomWaitSecond * 1000;

				writeToScreen("PRESS!", 1, 1, 1);
      
        currentState = BUTTON_WAITING;
      }

      if (currentState == BUTTON_WAITING && milliseconds > gameSessionEndTime) {
        currentState = END_SCREEN;
      }

      if (currentState == END_SCREEN) {
				if(redScore == 0 && greenScore == 0)
        unsigned int redScore = redPlayerPressedTime - gameSessionStartTime;
        unsigned int greenScore = greenPlayerPressedTime - gameSessionStartTime;
				{
					
			  } else if (redScore <= 100 && greenScore <= 100) {
					writeToScreen("WOW! You both failed.", 1, 1, 1);
        } else if (greenScore <= 100) {
					writeToScreen("RED: false start", 1, 1, 1);
					writeToScreen("GREEN WINS!", 1, 2, 0);
        } else if (greenScore <= 100) {
					writeToScreen("GREEN: false start", 1, 1, 1);
					writeToScreen("RED WINS!", 1, 2, 0);
        } else {
					char str[80];
					sprintf(str, "RED: %d ms", redScore);
					writeToScreen(str, 1, 1, 1);
					sprintf(str, "GREEN: %d ms", greenScore);
					writeToScreen(str, 1, 2, 0);
					writeToScreen(redScore < greenScore ? "RED WINS!": "GREEN WINS!", 1, 3, 0);
				}
				
				scoreDisplayEndTime = milliseconds + scoreShowTime;
        isScoreShowing = 1;
      }
    } else if (milliseconds >= scoreDisplayEndTime) {
			currentState = INITIALIZE;
			isScoreShowing = 0;
		}
  }
}
