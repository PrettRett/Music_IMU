/*
 * BLE_serial.h
 *
 *  Created on: 9 mar. 2018
 *      Author: Daniel
 */

#ifndef BLE_SERIAL_H_
#define BLE_SERIAL_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "list.h"
#include "timers.h"
#include "event_groups.h"
#include "portmacro.h"
#include "task.h"
#include "FreeRTOSconfig.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "stdbool.h"
#include "stdint.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "BNO055.h"

#define QUEUE_LENGTH 80
#define QUEUE_SIZE sizeof(uint8_t)

//funcion de freeRTOS
#define BLE_FLAG 0x01
#define DATA_SEND_FLAG 0X04
#define TRANS_END_FLAG 0X08
#define CALIB_END_FLAG 0X10
#ifdef USB_CONN
    #define USB_FLAG 0x02
    QueueHandle_t xRxedChars0, xCharsForTx0;
#else
    #define USB_FLAG 0x00
#endif

extern uint8_t mode_BNO;
extern SemaphoreHandle_t mut;
extern EventGroupHandle_t Signals_Comm, Signal_BNO;
QueueHandle_t xRxedChars1, xCharsForTx1;
uint8_t buttonPressed;


void BLE_serialTask(void *pvParameters);
void UARTBLEinit();

void UART1IntHandler();
void UART0IntHandler();

void ButtonStopHandler();

#endif /* BLE_SERIAL_H_ */
