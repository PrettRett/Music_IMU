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

//variables para el env�o de la trama

unsigned char end[]="\r\n";
unsigned char separation=';';
unsigned char NameVec[]="GAQPT";

void BLE_serialTask(void *pvParameters)
{
    unsigned char str[30];
    unsigned char comm[7][8]={"XREAD","XCALI","OK+CONN","OK+LOST","G\n","A\n","Q\n"};
    BLE_state=INIT;
    /*uint8_t com_count1=0;
    uint8_t com_count2=0;*/

    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x00);
    while(1)
    {
        EventBits_t aux=xEventGroupWaitBits(Signals, BLE_FLAG|USB_FLAG|DATA_SEND_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x10);
        if((aux & BLE_FLAG)==BLE_FLAG)
        {
#ifdef USB_CONN
            while(uxQueueMessagesWaiting(xRxedChars1)&&uxQueueSpacesAvailable(xCharsForTx0))
#else
            while(uxQueueMessagesWaiting(xRxedChars1))
#endif
            {
                xQueueReceive(xRxedChars1,&str,portMAX_DELAY);
#ifdef USB_CONN
                xQueueSend(xCharsForTx0,&str,portMAX_DELAY);
#endif
                /*  //Codigo en caso de que se reciba caracter a caracter, como no va a ser el caso,
                 *   se escoger� una opci�n m�s optimizada que permitir� tener una m�quina de estados
                if(str==comm[0][com_count1])
                {
                    com_count1++;
                    if(com_count1==5)
                    {
                        xEventGroupSetBits(Signals,READ_FLAG);
                        com_count1=0;
                    }
                }
                else if(com_count1>0)
                    com_count1=0;
                if(str==comm[1][com_count2])
                {
                    com_count2++;
                    if(com_count2==5)
                    {
                        xEventGroupSetBits(Signals,CALIB_FLAG);
                        com_count2=0;
                    }
                }
                else if(com_count2>0)
                    com_count2=0;*/

            }

            //
            // Disable the UART interrupt.  If we don't do this there is a race
            // condition which can cause the read index to be corrupted.
            //
            UARTIntDisable(UART1_BASE, UART_INT_TX);

#ifdef USB_CONN
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
#endif

            //
            // Reenable the UART interrupt.
            //
            MAP_UARTIntEnable(UART1_BASE, UART_INT_TX);
            switch(BLE_state)
            {
            case INIT:
                if(0==memcmp(str,comm[2],7))
                {
                    BLE_state=CONNECTED;
                }
                break;
            case CONNECTED:
                if(0==memcmp(str,comm[0],5))
                {
                    BLE_state=SENDING;
                    xEventGroupSetBits(Signals,READ_FLAG);
                }
                else if((0==memcmp(str,comm[1],5)))
                {
                    BLE_state=CALIBRATING;
                    xEventGroupSetBits(Signals,CALIB_FLAG);
                }
                else if((0==memcmp(str,comm[3],7)))
                {
                    BLE_state=INIT;
                }
                break;
            case SENDING:
                if(0==memcmp(str,comm[0],7))
                {
                    BLE_state=CONNECTED;
                    xEventGroupSetBits(Signals,READ_FLAG);
                }
                else if((0==memcmp(str,comm[4],2)))
                {

                }
                else if((0==memcmp(str,comm[5],2)))
                {

                }
                else if((0==memcmp(str,comm[6],2)))
                {

                }
                else if((0==memcmp(str,comm[3],7)))
                {
                    BLE_state=INIT;
                    xEventGroupSetBits(Signals,READ_FLAG);
                }
                break;
            case CALIBRATING:
            default:
                break;
            }
        }
#ifdef USB_CONN
        if((aux & USB_FLAG)==USB_FLAG)
        {
            while(uxQueueMessagesWaiting(xRxedChars0)&&uxQueueSpacesAvailable(xCharsForTx1))
            {
                xQueueReceive(xRxedChars0,&str,portMAX_DELAY);
                xQueueSend(xCharsForTx1,&str,portMAX_DELAY);

                if(str==comm[0][com_count1])
                {
                    com_count1++;
                    if(com_count1==5)
                    {
                        xEventGroupSetBits(Signals,READ_FLAG);
                        com_count1=0;
                    }
                }
                else if(com_count1>0)
                    com_count1=0;
                if(str==comm[1][com_count2])
                {
                    com_count2++;
                    if(com_count2==5)
                    {
                        xEventGroupSetBits(Signals,CALIB_FLAG);
                        com_count2=0;
                    }
                }
                else if(com_count2>0)
                    com_count2=0;

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
#endif
        if(aux & DATA_SEND_FLAG)
        {

            //
            // Disable the UART interrupt.  If we don't do this there is a race
            // condition which can cause the read index to be corrupted.
            //
            unsigned char *dataToSend;

            xSemaphoreTake(mut,portMAX_DELAY);
#ifdef USB_CONN
            UARTIntDisable(UART0_BASE, UART_INT_TX);
#else
            UARTIntDisable(UART1_BASE, UART_INT_TX);
#endif
            int i=0;

            //--------------anterior intento---------------
            /*
            int d=0;
            while(UARTSpaceAvail(UART0_BASE)&&(i<67) )
            {
                if(i%3!=2)
                    UARTCharPutNonBlocking(UART0_BASE,sensors_value.mult_read[d++]);
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
                    xQueueSend(xCharsForTx0,&(sensors_value.mult_read[d++]),portMAX_DELAY);
                else
                    xQueueSend(xCharsForTx0,&separation,portMAX_DELAY);
                i++;
            }
            xQueueSend(xCharsForTx0,&end[0],portMAX_DELAY);
            xQueueSend(xCharsForTx0,&end[1],portMAX_DELAY);*/

            //se inicia la transmisi�n, debo de enviar los vectores de Quaterion, de Gravity, de Acceleraci�n lineal y del tiempo
            //como la transmisi�n va a ser de 11 bytes por vector ( 1 para nombrar cual enviamos, 3 int16, 1 ';' por cada int16 y un "\n"),
            //excepto el quaterion que ser� de 14 y el tiempo que ser� de 7 bytes, por tanto, se enviar�n 45 bytes
            uint8_t send_uart=1;
            while(i<48)
            {
                if(i>=40)
                {
                    uint8_t cmp=i%40;
                    if(5==cmp)
                        dataToSend=&separation;
                    else if(0==cmp)
                        dataToSend=NameVec+4;
                    else if(5>cmp)
                    {
                        dataToSend=(uint8_t *)&read_time;
                        dataToSend=dataToSend+cmp-1;
                    }
                    else
                        dataToSend=end+cmp-6;

                }
                else if(i>=36)
                {
                    uint8_t cmp=i%36;
                    if(2==cmp)
                        dataToSend=&separation;
                    else if(0==cmp)
                        dataToSend=NameVec+3;
                    else if(1==cmp)
                        dataToSend=&buttonPressed;
                    else
                        dataToSend=end+1;

                }
                else if(i>=32)
                {
                    uint8_t cmp=i%32;
                    if(2==cmp)
                        dataToSend=&separation;
                    else if(2>cmp)
                        dataToSend=&(sensors_value.axis.QUAW_LSB) + cmp;
                    else
                        dataToSend=end+1;

                }
                else if((i%11)==0)
                {
                    dataToSend=&(NameVec[(i/11)]);
                }
                else if((i%11)>=1)
                {
                    uint8_t cmp=(i%11) - 1;
                    if(cmp>=9)
                        dataToSend=end+1;
                    else if(cmp%3==2)
                        dataToSend=&separation;
                    else
                    {
                        if(i/11==0)
                        {
                            dataToSend=&(sensors_value.axis.GRAVX_LSB) + cmp - cmp/3;
                        }
                        else if(i/11==1)
                        {
                            dataToSend=&(sensors_value.axis.LINX_LSB) + cmp - cmp/3;
                        }
                        else if(i/11==2)
                        {
                            dataToSend=&(sensors_value.axis.QUAX_LSB) + cmp - cmp/3;
                        }
                    }
                }/*//idea equivocada
                if(i==0)
                    BLEChangeChar("FFE1");
                else if(i==11)
                    BLEChangeChar("FFE2");
                else if(i==22)
                    BLEChangeChar("FFE3");
                else if(i==36)
                    BLEChangeChar("FFE4");
                else if(i==40)
                    BLEChangeChar("FFE5");*/
#ifdef USB_CONN
                if(send_uart&&UARTSpaceAvail(UART0_BASE))
                {
                    UARTCharPutNonBlocking(UART0_BASE,*dataToSend);
                }
                else
                {
                    send_uart=0;
                    BaseType_t xResult=xQueueSend(xCharsForTx0,dataToSend,configTICK_RATE_HZ*0.01);
                    //codigo extra para cuando se pierdan datos
                    if(xResult==errQUEUE_FULL)
                    {
                        i=48;
                        xEventGroupSetBits(Signals,NSENT_FLAG);
                        //avisamos cuando no se ha conseguido enviar
                    }
                }
#else
                if(send_uart&&UARTSpaceAvail(UART1_BASE))
                {
                    UARTCharPutNonBlocking(UART1_BASE,*dataToSend);
                }
                else
                {
                    send_uart=0;
                    BaseType_t xResult=xQueueSend(xCharsForTx1,dataToSend,configTICK_RATE_HZ*0.01);
                    //codigo extra para cuando se pierdan datos
                    if(xResult==errQUEUE_FULL)
                    {
                        i=48;
                        xEventGroupSetBits(Signals,NSENT_FLAG);
                        //avisamos cuando no se ha conseguido enviar
                    }
                }
#endif
                i++;

            }


            //
            // Reenable the UART interrupt.
            //
            xSemaphoreGive(mut);
#ifdef USB_CONN
            MAP_UARTIntEnable(UART0_BASE, UART_INT_TX);
#else
            MAP_UARTIntEnable(UART1_BASE, UART_INT_TX);
#endif

        }

    }
}
/*//Idea equivocada
void BLEChangeChar(uint8_t *C_ar)
{
    uint8_t str[] = "AT+CHAR0x0000";
    int i;
    for(i=0;i<4;i++)
        str[9+i]=C_ar[i];
    int send_uart=1;
    for(i=0;i<sizeof(str);i++)
    {
            if(send_uart&&UARTSpaceAvail(UART1_BASE))
            {
                UARTCharPutNonBlocking(UART1_BASE,str+i);
            }
            else
            {
                send_uart=0;
                BaseType_t xResult=xQueueSend(xCharsForTx1,str+i,configTICK_RATE_HZ*0.01);
                //codigo extra para cuando se pierdan datos
                if(xResult==errQUEUE_FULL)
                {
                    break;
                }
            }
    }
}*/

void BLESendBNOInfo(uint8_t initial)
{
    uint8_t *dataToSend, *initial_reg, cmp;
    uint8_t i=0;
    uint8_t fin=20;
    uint8_t send_uart=1;
    if(initial=='G')
    {
        initial_reg=&(sensors_value.axis.GRAVX_LSB);
        fin=23;
    }
    else if(initial=='A')
        initial_reg=&(sensors_value.axis.LINX_LSB);
    else if(initial=='Q')
        initial_reg=&(sensors_value.axis.QUAX_LSB);
    while(i<fin)
    {
        if(0==i)
            dataToSend=&initial;
        else if(i<(10+(fin%20)))
        {
            cmp = i%3;
            if(0==cmp)
                dataToSend=&separation;
            else
                dataToSend=initial_reg++;
        }
        else if(i<(16+(fin%20)))
        {
            cmp=i%(10+(fin%20));
            if(0==cmp)
                dataToSend=NameVec+4;
            else if(5>cmp)
            {
                dataToSend=(uint8_t *)&read_time;
                dataToSend=dataToSend+cmp-1;
            }
            else
                dataToSend=&separation;
        }
        else
        {
            cmp=cmp=i%(16+(fin%20));
            if(0==cmp)
                dataToSend=NameVec+3;
            else if(2>cmp)
                dataToSend=&buttonPressed;
            else
                dataToSend=&separation;
        }

        if(send_uart&&UARTSpaceAvail(UART1_BASE))
        {
            UARTCharPutNonBlocking(UART1_BASE,*dataToSend);
        }
        else
        {
            send_uart=0;
            BaseType_t xResult=xQueueSend(xCharsForTx1,dataToSend,configTICK_RATE_HZ*0.01);
            //codigo extra para cuando se pierdan datos
            if(xResult==errQUEUE_FULL)
            {
                i=48;
                xEventGroupSetBits(Signals,NSENT_FLAG);
                //avisamos cuando no se ha conseguido enviar
            }
        }

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

    //--------------------Habilitar el ENABLE del HM-10-------------------
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x10);

    //boton para se�alizar el movimiento
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
    GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4);
    GPIOIntEnable(GPIO_PORTF_BASE,GPIO_PIN_4);
    IntEnable(INT_GPIOF);
    buttonPressed=!(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)>>4);
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

void ButtonStopHandler()
{
    buttonPressed=!(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)>>4);
    GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4);
}
