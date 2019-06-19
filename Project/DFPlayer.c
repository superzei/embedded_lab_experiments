/*
 * DFPlayer.c
 *
 *  Created on: Jul 3, 2017
 *      Author: minht57
 */

#include "DFPlayer.h"

static uint8_t send_buf[10] = {0x7E, 0xFF, 06, 00, 00, 00, 00, 00, 00, 0xEF};
static uint8_t recv_buf[10 - 1];

#define TIME_WAIT_FOR_CMD       10
#define MAX_BUF_DATA_RECV       50
uint8_t DFP_Buf[MAX_BUF_DATA_RECV];
uint16_t DFP_ReadIdx = 0;
uint16_t DFP_WriteIdx = 0;
uint16_t u16_avail_byte = 0;

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(DFPlayer_UART_BASE, true); //get interrupt status
    UARTIntClear(DFPlayer_UART_BASE, ui32Status); //clear the asserted interrupts

    while(UARTCharsAvail(DFPlayer_UART_BASE)) //loop while there are chars
    {
        if(((DFP_WriteIdx == (MAX_BUF_DATA_RECV -1))&&(DFP_ReadIdx != 0))||
           ((DFP_WriteIdx != (MAX_BUF_DATA_RECV -1))&&((DFP_WriteIdx+1) != DFP_ReadIdx)))
        {
            DFP_Buf[DFP_WriteIdx++] = UARTCharGetNonBlocking(DFPlayer_UART_BASE);
            u16_avail_byte++;
            DFP_WriteIdx %= MAX_BUF_DATA_RECV;
        }
    }
}

uint16_t UART_DFP_QueryData(void){
  return (u16_avail_byte);
}

void UART_DFP_Read(uint8_t * buf, uint16_t len){
    uint16_t idx;
    if(UART_DFP_QueryData() >= len){
        for(idx = 0; idx < len; idx++){
            if (DFP_ReadIdx != DFP_WriteIdx)
            {
              *(buf + idx) = DFP_Buf[DFP_ReadIdx++];
              if (u16_avail_byte)
              {
                u16_avail_byte--;
              }
            }
            if(DFP_ReadIdx >= MAX_BUF_DATA_RECV){
                DFP_ReadIdx = 0;
            }
        }
    }
}

void DFP_Init(void){
    //Init UART for DFPlayer
    SysCtlPeripheralEnable(DFPlayer_UART);
    SysCtlPeripheralEnable(DFPlayer_UART_GPIO);

    GPIOPinConfigure(DFPlayer_UART_RX);
    GPIOPinConfigure(DFPlayer_UART_TX);
    GPIOPinTypeUART(DFPlayer_GPIO_BASE, DFPlayer_GPIO_PIN);

    UARTConfigSetExpClk(DFPlayer_UART_BASE, SysCtlClockGet(), DFPlayer_Baudrate, DFPlayer_UART_Config);

    UARTIntRegister(DFPlayer_UART_BASE, &UARTIntHandler);
    UARTIntEnable(DFPlayer_UART_BASE, DFPlayer_UART_INT_Config); //only enable RX and TX interrupts
    IntEnable(DFPlayer_UART_INT); //enable the UART interrupt
    IntMasterEnable(); //enable processor interrupts
}

void delay (uint16_t ms)
{
    uint16_t u16_idx;
    for(u16_idx = 0; u16_idx < ms; u16_idx++)
    {
        SysCtlDelay(SysCtlClockGet()/3000);
    }
}

static void fill_uint16_bigend (uint8_t *thebuf, uint16_t data) {
    *thebuf =   (uint8_t)(data>>8);
    *(thebuf+1) =   (uint8_t)data;
}


//calc checksum (1~6 byte)
uint16_t mp3_get_checksum (uint8_t *thebuf) {
    uint16_t sum = 0;
    int8_t i;
    for (i=1; i<7; i++) {
        sum += thebuf[i];
    }
    return -sum;
}

//fill checksum to send_buf (7~8 byte)
void mp3_fill_checksum () {
    uint16_t checksum = mp3_get_checksum (send_buf);
    fill_uint16_bigend (send_buf+7, checksum);
}

//
void mp3_send_cmd (uint8_t cmd, uint16_t high_arg, uint16_t low_arg) {
    int8_t i;

    send_buf[3] = cmd;

    send_buf[5] = high_arg;
    send_buf[6] = low_arg;

    mp3_fill_checksum ();
    for (i=0; i<10; i++) {
        UARTCharPut(DFPlayer_UART_BASE, send_buf[i]);
    }

    delay(10);
//    _last_call = millis();
}

// Wait and receive replay for specific command
int16_t mp3_recv_cmd (uint8_t wait) {
    int8_t i8_time_wait_for_cmd = TIME_WAIT_FOR_CMD;
    uint8_t received = 0;
    do {
        UART_DFP_Read(&received,1);
    }
    while ((received != 0x7e) && (i8_time_wait_for_cmd-- != 0));

    if(i8_time_wait_for_cmd == 0) {
        return 0xffff;
    }

    UART_DFP_Read(recv_buf,9);

    received = recv_buf[3 - 1];

    if (received == 0x40) {
        // Error responce
        return 0xffff;
    }
    else if ((wait != 0) && (wait == received)) {
        return (((int16_t)recv_buf[5 - 1]) *0xff + ((int16_t)recv_buf[6 - 1]));
    }
    return 0xffff;
}

//
void DFP_play_physical (uint16_t num) {
    mp3_send_cmd (0x03, 0, num);
}

//
void DFP_play (void) {
    mp3_send_cmd(0x0d, 0 ,0);
}

//
void DFP_pause (void) {
    mp3_send_cmd (0x0e, 0 ,0);
}

//
void DFP_stop (void) {
    mp3_send_cmd (0x16, 0, 0);
}

//play mp3 file in mp3 folder in your tf card
void DFP_play_folder (uint8_t num) {
    mp3_send_cmd (0x12, 0, num);
}

//
void DFP_next (void) {
    mp3_send_cmd (0x01, 0, 0);
}

//
void DFP_prev (void) {
    mp3_send_cmd (0x02, 0, 0);
}

//0x06 set volume 0-30
void DFP_set_volume (uint8_t volume) {
    mp3_send_cmd (0x06, 0, volume);
}

//0x07 set EQ0/1/2/3/4/5    Normal/Pop/Rock/Jazz/Classic/Bass
void DFP_set_EQ (uint8_t eq) {
    mp3_send_cmd (0x07, 0, eq);
}

//0x09 set device 1/2/3/4/5 U/SD/AUX/SLEEP/FLASH
void DFP_set_device (uint8_t device) {
    mp3_send_cmd (0x09, 0, device);
}

//
void DFP_sleep (void) {
    mp3_send_cmd (0x0a, 0, 0);
}

//
void DFP_reset (void) {
    mp3_send_cmd (0x0c, 0, 0);
}

//
void DFP_get_state (void) {
    mp3_send_cmd (0x42, 0, 0);
}

// Wait for mp3_get_state reply
int16_t mp3_wait_state (void) {
    return mp3_recv_cmd(0x42);
}

//
void DFP_get_volume (void) {
    mp3_send_cmd (0x43, 0, 0);
}

// Wait for mp3_get_volume reply
int16_t mp3_wait_volume (void) {
    return mp3_recv_cmd(0x43);
}

//
void DFP_get_u_sum (void) {
    mp3_send_cmd (0x47, 0, 0);
}

// Wait for mp3_get_u_sum reply
int16_t mp3_wait_u_sum (void) {
    return mp3_recv_cmd(0x47);
}

//
void DFP_get_tf_sum (void) {
    mp3_send_cmd (0x48, 0, 0);
}
// Wait for mp3_get_tf_sum reply
int16_t DFP_wait_tf_sum (void) {
    return mp3_recv_cmd(0x48);
}

//
void DFP_get_flash_sum (void) {
    mp3_send_cmd (0x49, 0, 0);
}

// Wait for mp3_get_flash_sum reply
int16_t mp3_wait_flash_sum (void) {
    return mp3_recv_cmd(0x49);
}

//
void DFP_get_tf_current (void) {
    mp3_send_cmd (0x4c, 0, 0);
}

// Wait for mp3_get_tf_current reply
int16_t mp3_wait_tf_current (void) {
    return mp3_recv_cmd(0x4c);
}

//
void DFP_get_u_current (void) {
    mp3_send_cmd (0x4b, 0, 0);
}

// Wait for mp3_get_u_current reply
int16_t mp3_wait_u_current(void) {
    return mp3_recv_cmd(0x4b);
}

//
void DFP_get_flash_current (void) {
    mp3_send_cmd (0x4d, 0, 0);
}

// Wait for mp3_get_flash_current reply
int16_t mp3_wait_flash_current (void) {
    return mp3_recv_cmd(0x4d);
}

//
void DFP_single_loop (uint8_t state) {
    mp3_send_cmd (0x19, 0, !state);
}

//add
void DFP_single_play (uint16_t num) {
    DFP_play_folder (num);
    delay (10);
    DFP_single_loop (1);
    //mp3_send_cmd (0x19, !state);
}

//
void DFP_DAC (uint8_t state) {
    mp3_send_cmd (0x1a, 0, !state);
}

//
void DFP_random_play (void) {
    mp3_send_cmd (0x18, 0, 0);
}

// Query total file numbers of a folder
void DFP_get_folder_sum (uint8_t folder) {
    mp3_send_cmd (0x4E, 0, folder);
}

// Wait for mp3_get_folder_sum reply
int16_t mp3_wait_folder_sum (void) {
    return mp3_recv_cmd(0x4E);
}

// Play mp3 file in selected folder
void DFP_play_file_in_folder (uint8_t folder, uint32_t num) {
    mp3_send_cmd (0x0f, folder, num);
}

/* END OF FILE*/
