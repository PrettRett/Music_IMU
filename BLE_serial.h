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

//funcion de freeRTOS
#define BLE_FLAG 0x01

EventGroupHandle_t Serials;

void BLE_serialTask(void *pvParameters);

void UART0IntHandler();


#endif /* BLE_SERIAL_H_ */
