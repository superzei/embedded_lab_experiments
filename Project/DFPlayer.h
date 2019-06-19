/*
 * DFPlayer.h
 *
 *  Created on: Jul 3, 2017
 *      Author: minht57
 */

#ifndef DFPLAYER_DFPLAYER_H_
#define DFPLAYER_DFPLAYER_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

//#define DEBUG

#ifdef DEBUG

#define DFPlayer_UART               SYSCTL_PERIPH_UART0
#define DFPlayer_UART_GPIO          SYSCTL_PERIPH_GPIOA
#define DFPlayer_UART_RX            GPIO_PA0_U0RX
#define DFPlayer_UART_TX            GPIO_PA1_U0TX
#define DFPlayer_GPIO_BASE          GPIO_PORTA_BASE
#define DFPlayer_GPIO_PIN           (GPIO_PIN_0 | GPIO_PIN_1)
#define DFPlayer_UART_BASE          UART0_BASE
#define DFPlayer_Baudrate           9600
#define DFPlayer_UART_Config        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define DFPlayer_UART_INT           INT_UART0
#define DFPlayer_UART_INT_Config    (UART_INT_RX | UART_INT_RT)

#else

#define DFPlayer_UART               SYSCTL_PERIPH_UART3
#define DFPlayer_UART_GPIO          SYSCTL_PERIPH_GPIOC
#define DFPlayer_UART_RX            GPIO_PC6_U3RX
#define DFPlayer_UART_TX            GPIO_PC7_U3TX
#define DFPlayer_GPIO_BASE          GPIO_PORTC_BASE
#define DFPlayer_GPIO_PIN           (GPIO_PIN_6 | GPIO_PIN_7)
#define DFPlayer_UART_BASE          UART3_BASE
#define DFPlayer_Baudrate           9600
#define DFPlayer_UART_Config        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define DFPlayer_UART_INT           INT_UART3
#define DFPlayer_UART_INT_Config    (UART_INT_RX | UART_INT_RT)

#endif
// 7E FF 06 0F 00 01 01 xx xx EF
// 0    ->  7E is start code
// 1    ->  FF is version
// 2    ->  06 is length
// 3    ->  0F is command
// 4    ->  00 is no receive
// 5~6  ->  01 01 is argument
// 7~8  ->  checksum = 0 - ( FF+06+0F+00+01+01 )
// 9    ->  EF is end code

void DFP_Init (void);
void DFP_play_physical (uint16_t num);
void DFP_play (void);
void DFP_pause (void);
void DFP_stop (void);
void DFP_play_folder (uint8_t num);
void DFP_next (void);
void DFP_prev (void);
void DFP_set_volume (uint8_t volume);
void DFP_set_EQ (uint8_t eq);
void DFP_set_device (uint8_t device);
void DFP_sleep (void);
void DFP_reset (void);
void DFP_get_state (void);
int16_t mp3_wait_state (void);
void DFP_get_volume (void);
int16_t mp3_wait_volume (void);
void DFP_get_u_sum (void);
int16_t mp3_wait_u_sum (void);
void DFP_get_tf_sum (void);
int16_t DFP_wait_tf_sum (void);
void DFP_get_flash_sum (void);
int16_t mp3_wait_flash_sum (void);
void DFP_get_tf_current (void);
int16_t mp3_wait_tf_current (void);
void DFP_get_u_current (void);
int16_t mp3_wait_u_current(void);
void DFP_get_flash_current (void);
int16_t mp3_wait_flash_current (void);
void DFP_single_loop (uint8_t state);
void DFP_single_play (uint16_t num);
void DFP_DAC (uint8_t state);
void DFP_random_play (void);
void DFP_get_folder_sum (uint8_t folder);
int16_t mp3_wait_folder_sum (void);
void DFP_play_file_in_folder (uint8_t folder, uint32_t num);

#endif /* DFPLAYER_DFPLAYER_H_ */
