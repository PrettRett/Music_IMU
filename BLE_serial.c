/*
 * BLE_serial.c
 *
 *  Created on: 9 mar. 2018
 *      Author: Daniel
 */

#include "BLE_serial.h"
#include "timers.h"
#include "event_groups.h"
#include "driverlib/uart.h"

void BLE_serialTask(void *pvParameters)
{
    char str[32];
    while(1)
    {
        xEventGroupWaitBits(Serials, BLE_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        int i=0;
        while(UARTCharsAvail(UART1_BASE))
        {
            str[i]=UARTCharGet(UART1_BASE);
            i++;
            if(i>=32)
                break;
        }
        int d;
        for(d=0; d<i; d++)
        {
            if(!UARTCharPutNonBlocking(UART0_BASE, str[d]))
            {
                while(!UARTSpaceAvail(UART0_BASE));
                UARTCharPut(UART0_BASE,(unsigned char) 'E');
                break;
            }
        }
    }
}

void UART1IntHandler()
{
    if(UARTIntStatus(UART0_BASE,UART_INT_RX)!=UART_INT_RX)
        while(1);
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken=pdFALSE;
    xResult = xEventGroupSetBitsFromISR(
                                  Serials,   /* The event group being updated. */
                                  BLE_FLAG, /* The bits being set. */
                                  &xHigherPriorityTaskWoken );
    if(xResult != pdFAIL)
    {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    UARTIntClear(UART0_BASE,UART_INT_RX);
}
