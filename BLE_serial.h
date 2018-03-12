/*
 * BLE_serial.h
 *
 *  Created on: 9 mar. 2018
 *      Author: Daniel
 */

#ifndef BLE_SERIAL_H_
#define BLE_SERIAL_H_


//funcion de freeRTOS
void BLE_serialTask(void *pvParameters);

void UART0IntHandler();


#endif /* BLE_SERIAL_H_ */
