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
        EventBits_t aux=xEventGroupWaitBits(Signals, BLE_FLAG|USB_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        int i=0;
#ifdef USB_CONN
        if((aux & BLE_FLAG)==BLE_FLAG)
        {
            while(UARTCharsAvail(UART1_BASE))
            {
                str[i]=UARTCharGet(UART1_BASE);
                i++;
                if(i>=32)
                    break;
            }
            UARTIntEnable(UART1_BASE, UART_INT_RX|UART_INT_RT);
            int d;
            for(d=0; d<i; d++)
            {
                if(!UARTCharPutNonBlocking(UART0_BASE, str[d]))
                {
                    while(!UARTSpaceAvail(UART0_BASE));
                    UARTCharPut(UART0_BASE,str[d]);
                    break;
                }
            }
        }
        if((aux & USB_FLAG)==USB_FLAG)
        {
            while(UARTCharsAvail(UART0_BASE))
            {
                str[i]=UARTCharGet(UART0_BASE);
                i++;
                if(i>=32)
                    break;
            }
            UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_RT);
            int d;
            for(d=0; d<i; d++)
            {
                if(!UARTCharPutNonBlocking(UART1_BASE, str[d]))
                {
                    while(!UARTSpaceAvail(UART1_BASE));
                    UARTCharPut(UART1_BASE,str[d]);
                    break;
                }
            }
        }
#endif
    }
}

void UARTBLEinit()
{

    //Event_groups para avisar a las funciones de los serial
    Signals=xEventGroupCreate();

    /*Rx1Queue=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);
    Rx0Queue=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);*/

    //
    // Inicializa la UARTy la configura a 115.200 bps, 8-N-1 .
    //se usa para mandar y recibir mensajes y comandos por el puerto serie
    // Mediante un programa terminal como gtkterm, putty, cutecom, etc...
    //

#ifdef USB_CONN
    //---------------------UART0------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Esta funcion habilita la interrupcion de la UART y le da la prioridad adecuada si esta activado el soporte para FreeRTOS
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);   //La UART tiene que seguir funcionando aunque el micro esta dormido
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOA);
    UARTClockSourceSet(UART0_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),9600,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);

    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UARTIntClear(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTFIFOEnable(UART0_BASE);
    IntEnable(INT_UART0);
    UARTEnable(UART0_BASE);
#endif

    //---------------------UART0------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Esta funcion habilita la interrupcion de la UART y le da la prioridad adecuada si esta activado el soporte para FreeRTOS
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART1);   //La UART tiene que seguir funcionando aunque el micro esta dormido
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOB);
    UARTClockSourceSet(UART1_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART1_BASE,SysCtlClockGet(),115200,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);

    UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UARTIntClear(UART1_BASE, UART_INT_RX|UART_INT_RT);
    UARTIntEnable(UART1_BASE, UART_INT_RX|UART_INT_RT);
    UARTFIFOEnable(UART1_BASE);
    IntEnable(INT_UART1);
    UARTEnable(UART1_BASE);
}


void UART1IntHandler()
{
    uint32_t aux;
    if( (aux=UARTIntStatus(UART1_BASE,pdTRUE))&(UART_INT_RX|UART_INT_RT))
    {
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken=pdFALSE;
        xResult = xEventGroupSetBitsFromISR(
                                  Serials,   /* The event group being updated. */
                                  BLE_FLAG, /* The bits being set. */
                                  &xHigherPriorityTaskWoken );
        if(xResult != pdFAIL)
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        UARTIntDisable(UART1_BASE, UART_INT_RX|UART_INT_RT);
    }
    UARTIntClear(UART1_BASE,aux);
}

#ifdef USB_CONN
void UART0IntHandler()
{
    uint32_t aux;
    if( (aux=UARTIntStatus(UART0_BASE,pdTRUE))&(UART_INT_RX|UART_INT_RT))
    {
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken=pdFALSE;
        xResult = xEventGroupSetBitsFromISR(
                                  Serials,   /* The event group being updated. */
                                  USB_FLAG, /* The bits being set. */
                                  &xHigherPriorityTaskWoken );
        if(xResult != pdFAIL)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
        UARTIntDisable(UART0_BASE, UART_INT_RX|UART_INT_RT);
    }
    UARTIntClear(UART0_BASE,aux);
}
#else
void UART0IntHandler()
{
    UARTIntClear(UART0_BASE,(UARTIntStatus(UART0_BASE,pdTRUE)));
}
#endif
