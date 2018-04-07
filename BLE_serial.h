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

#define USB_CONN
#define QUEUE_LENGTH 12
#define QUEUE_SIZE sizeof(uint8_t)

//funcion de freeRTOS
#define BLE_FLAG 0x01
#ifdef USB_CONN
    #define USB_FLAG 0x02
#else
    #define USB_FLAG 0x00
#endif

extern EventGroupHandle_t Signals;

//QueueHandle_t Rx0Queue, Rx0Queue;

void BLE_serialTask(void *pvParameters);
void UARTBLEinit();

void UART1IntHandler();
void UART0IntHandler();

#endif /* BLE_SERIAL_H_ */
