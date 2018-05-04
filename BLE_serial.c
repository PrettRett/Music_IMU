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
    unsigned char str;
    unsigned char comm[]="XREAD";
    uint8_t com_count=0;

    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x00);
    while(1)
    {
        EventBits_t aux=xEventGroupWaitBits(Signals, BLE_FLAG|USB_FLAG|DATA_SEND_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x10);
#ifdef USB_CONN
        if((aux & BLE_FLAG)==BLE_FLAG)
        {
            while(uxQueueMessagesWaiting(xRxedChars1)&&uxQueueSpacesAvailable(xCharsForTx0))
            {
                xQueueReceive(xRxedChars1,&str,portMAX_DELAY);
                xQueueSend(xCharsForTx0,&str,portMAX_DELAY);

                if(str==comm[com_count])
                {
                    com_count++;
                    if(com_count==5)
                    {
                        xEventGroupSetBits(Signals,0x20);
                        com_count=0;
                    }
                }
                else if(com_count>0)
                    com_count=0;

            }
            //
            // Disable the UART interrupt.  If we don't do this there is a race
            // condition which can cause the read index to be corrupted.
            //
            UARTIntDisable(UART1_BASE, UART_INT_TX);

            //
            // Yes - take some characters out of the transmit buffer and feed
            // them to the UART transmit FIFO.
            //
            while(UARTSpaceAvail(UART0_BASE) && (uxQueueMessagesWaiting(xCharsForTx0)))
            {
                uint8_t data;
                xQueueReceive(xCharsForTx0,&data,portMAX_DELAY);
                UARTCharPutNonBlocking(UART0_BASE,data);
            }

            //
            // Reenable the UART interrupt.
            //
            MAP_UARTIntEnable(UART1_BASE, UART_INT_TX);
        }
        if((aux & USB_FLAG)==USB_FLAG)
        {
            while(uxQueueMessagesWaiting(xRxedChars0)&&uxQueueSpacesAvailable(xCharsForTx1))
            {
                xQueueReceive(xRxedChars0,&str,portMAX_DELAY);
                xQueueSend(xCharsForTx1,&str,portMAX_DELAY);

                if(str==comm[com_count])
                {
                    com_count++;
                    if(com_count==5)
                    {
                        xEventGroupSetBits(Signals,0x20);
                        com_count=0;
                    }
                }
                else if(com_count>0)
                    com_count=0;

            }
            //
            // Disable the UART interrupt.  If we don't do this there is a race
            // condition which can cause the read index to be corrupted.
            //
            UARTIntDisable(UART0_BASE, UART_INT_TX);

            //
            // Yes - take some characters out of the transmit buffer and feed
            // them to the UART transmit FIFO.
            //
            while(UARTSpaceAvail(UART1_BASE) && (uxQueueMessagesWaiting(xCharsForTx1)))
            {
                uint8_t data;
                xQueueReceive(xCharsForTx1,&data,portMAX_DELAY);
                UARTCharPutNonBlocking(UART1_BASE,data);
            }

            //
            // Reenable the UART interrupt.
            //
            MAP_UARTIntEnable(UART0_BASE, UART_INT_TX);
        }
        if(aux & DATA_SEND_FLAG)
        {


            //
            // Disable the UART interrupt.  If we don't do this there is a race
            // condition which can cause the read index to be corrupted.
            //
            UARTIntDisable(UART0_BASE, UART_INT_TX);
            unsigned char end[]="\r\n";
            unsigned char separation=';';

            xSemaphoreTake(mut,portMAX_DELAY);
            int i=0;
            int d=0;
            while(UARTSpaceAvail(UART0_BASE)&&(i<67) )
            {
                if(i%3!=2)
                    UARTCharPutNonBlocking(UART0_BASE,mult_read[d++]);
                else
                    UARTCharPutNonBlocking(UART0_BASE,separation);
                i++;
            }

            //
            // Yes - take some characters out of the transmit buffer and feed
            // them to the UART transmit FIFO.
            //
            while(i<67)
            {
                if(i%3!=2)
                    xQueueSend(xCharsForTx0,&mult_read[d++],portMAX_DELAY);
                else
                    xQueueSend(xCharsForTx0,&separation,portMAX_DELAY);
                i++;
            }
            xQueueSend(xCharsForTx0,&end[0],portMAX_DELAY);
            xQueueSend(xCharsForTx0,&end[1],portMAX_DELAY);


            //
            // Reenable the UART interrupt.
            //
            xSemaphoreGive(mut);
            MAP_UARTIntEnable(UART0_BASE, UART_INT_TX);

        }

#endif
    }
}

void UARTBLEinit()
{

    //Event_groups para avisar a las funciones de los serial
    Signals=xEventGroupCreate();

    xCharsForTx1=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);
    xRxedChars1=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);
    mut=xSemaphoreCreateMutex();

    //
    // Inicializa la UARTy la configura a 115.200 bps, 8-N-1 .
    //se usa para mandar y recibir mensajes y comandos por el puerto serie
    // Mediante un programa terminal como gtkterm, putty, cutecom, etc...
    //

#ifdef USB_CONN
    xRxedChars0=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);
    xCharsForTx0=xQueueCreate(QUEUE_LENGTH, QUEUE_SIZE);
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
    UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),115200,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);

    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UARTIntClear(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTFIFOEnable(UART0_BASE);
    IntEnable(INT_UART0);
    UARTEnable(UART0_BASE);
#endif

    //---------------------UART1------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Esta funcion habilita la interrupcion de la UART y le da la prioridad adecuada si esta activado el soporte para FreeRTOS
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART1);   //La UART tiene que seguir funcionando aunque el micro esta dormido
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOB);
    UARTClockSourceSet(UART1_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART1_BASE,SysCtlClockGet(),9600,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);

    UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UARTIntClear(UART1_BASE, UART_INT_RX|UART_INT_RT|UART_INT_TX);
    UARTIntEnable(UART1_BASE, UART_INT_RX|UART_INT_RT|UART_INT_TX);
    UARTFIFOEnable(UART1_BASE);
    IntEnable(INT_UART1);
    UARTEnable(UART1_BASE);

    //--------------------Habilitarel ENABLE del HM-10-------------------
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x10);
    while(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_5));
}


void UART1IntHandler()
{
    uint32_t ui32Ints;
    int8_t cChar;
    int32_t i32Char;
    portBASE_TYPE xHigherPriorityTaskWoken=false;


    //
    // Get and clear the current interrupt source(s)
    //
    ui32Ints = MAP_UARTIntStatus(UART1_BASE, true);
    MAP_UARTIntClear(UART1_BASE, ui32Ints);

    //
    // Are we being interrupted because the TX FIFO has space available?
    //
    if(ui32Ints & UART_INT_TX)
    {
        //
        // Move as many bytes as we can into the transmit FIFO.
        //
        while(MAP_UARTSpaceAvail(UART1_BASE) && !xQueueIsQueueEmptyFromISR(xCharsForTx1))
        {
            uint8_t data;
            xQueueReceiveFromISR(xCharsForTx1,&data,&xHigherPriorityTaskWoken);
            MAP_UARTCharPutNonBlocking(UART1_BASE,data);
        }

        //
        // If the output buffer is empty, turn off the transmit interrupt.
        //
        if(xQueueIsQueueEmptyFromISR(xCharsForTx1))
        {
            MAP_UARTIntDisable(UART1_BASE, UART_INT_TX);
        }
    }

    //
    // Are we being interrupted due to a received character?
    //
    if(ui32Ints & (UART_INT_RX | UART_INT_RT))
    {
        //
        // Get all the available characters from the UART.
        //
        while(MAP_UARTCharsAvail(UART1_BASE))
        {
            //
            // Read a character
            //
            i32Char = MAP_UARTCharGetNonBlocking(UART1_BASE);
            cChar = (unsigned char)(i32Char & 0xFF);
            xEventGroupSetBitsFromISR(Signals,   /* The event group being updated. */
                                      BLE_FLAG, /* The bits being set. */
                                      &xHigherPriorityTaskWoken );
            //
            // If there is space in the receive buffer, put the character
            // there, otherwise throw it away.
            //

            xQueueSendFromISR(xRxedChars1,&cChar,&xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

#ifdef USB_CONN
void UART0IntHandler()
{
        uint32_t ui32Ints;
        int8_t cChar;
        int32_t i32Char;
        portBASE_TYPE xHigherPriorityTaskWoken=false;


        //
        // Get and clear the current interrupt source(s)
        //
        ui32Ints = MAP_UARTIntStatus(UART0_BASE, true);
        MAP_UARTIntClear(UART0_BASE, ui32Ints);

        //
        // Are we being interrupted because the TX FIFO has space available?
        //
        if(ui32Ints & UART_INT_TX)
        {
            //
            // Move as many bytes as we can into the transmit FIFO.
            //
            while(MAP_UARTSpaceAvail(UART0_BASE) && !xQueueIsQueueEmptyFromISR(xCharsForTx0))
            {
                uint8_t data;
                xQueueReceiveFromISR(xCharsForTx0,&data,&xHigherPriorityTaskWoken);
                MAP_UARTCharPutNonBlocking(UART0_BASE,data);
            }

            //
            // If the output buffer is empty, turn off the transmit interrupt.
            //
            if(xQueueIsQueueEmptyFromISR(xCharsForTx0))
            {
                MAP_UARTIntDisable(UART0_BASE, UART_INT_TX);
            }
        }

        //
        // Are we being interrupted due to a received character?
        //
        if(ui32Ints & (UART_INT_RX | UART_INT_RT))
        {
            //
            // Get all the available characters from the UART.
            //
            while(MAP_UARTCharsAvail(UART0_BASE))
            {
                //
                // Read a character
                //
                i32Char = MAP_UARTCharGetNonBlocking(UART0_BASE);
                cChar = (unsigned char)(i32Char & 0xFF);
                xEventGroupSetBitsFromISR(
                                                Signals,   /* The event group being updated. */
                                                USB_FLAG, /* The bits being set. */
                                                &xHigherPriorityTaskWoken );

                //
                // If there is space in the receive buffer, put the character
                // there, otherwise throw it away.
                //

                xQueueSendFromISR(xRxedChars0,&cChar,&xHigherPriorityTaskWoken);
            }
        }

        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
#else
void UART0IntHandler()
{
    UARTIntClear(UART0_BASE,(UARTIntStatus(UART0_BASE,pdTRUE)));
}
#endif
