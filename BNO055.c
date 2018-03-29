/*
 * BNO055.c
 *
 *  Created on: 27 mar. 2018
 *      Author: Daniel
 */

#include "BNO055.h"

void BNO_COMM(void *pvParameters)
{
    while(1)
    {
        EventBits_t aux=xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        switch (g_CurrState)
        {
            case BNO_INIT:
                g_PrevState = g_CurrState;
                g_CurrState = I2C_OP_TXADDR;

                I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);

            break;


            case BNO_CONF:
                g_PrevState = g_CurrState;

            /* If Address has been NAK'ed then go to stop state */
                if(aux & I2C_MASTER_INT_NACK)
                {
                    g_CurrState = I2C_OP_STOP;
                }
            /* Based on the direction move to the appropriate state of Transmit or Receive */
                else if(!g_I2CDirection)
                {
                    g_CurrState = I2C_OP_TXDATA;
                }
                else
                {
                    g_CurrState = I2C_OP_RXDATA;
                }

                I2CMasterDataPut(I2C0_BASE, (0x00));
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            break;


            case BNO_RDY:
                g_PrevState = g_CurrState;


            /* If Address or Data has been NAK'ed then go to stop state
               If a Stop condition is seen due to number of bytes getting
               done then move to STOP state
            */
                if(aux & I2C_MASTER_INT_NACK)
                {
                    g_CurrState = I2C_OP_STOP;
                }
                else if(aux & I2C_MASTER_INT_STOP)       /* query: what is purpose */
                {
                    g_CurrState = I2C_OP_STOP;
                }
                else
                {
                    I2CMasterDataPut(I2C0_BASE, g_ui8MasterTxData[g_ui8MasterBytes++]);
                    if(g_ui8MasterBytes == g_ui8MasterBytesLength)
                    {
                        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
                    }
                    else
                    {
                        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
                    }

                }

            break;


            case BNO_READ:
                g_PrevState = g_CurrState;

                /*
                 If Address has been NAK'ed then go to stop state
                 If a Stop condition is seen due to number of bytes getting
                 done then move to STOP state and read the last data byte
                */
                if(aux & I2C_MASTER_INT_NACK)
                {
                    g_CurrState = I2C_OP_STOP;
                }
                else if(aux & I2C_MASTER_INT_STOP)
                {
                    g_CurrState = I2C_OP_STOP;
                    g_ui8MasterRxData[g_ui8MasterBytes++] = I2CMasterDataGet(0x7d);
                }
                else
                {
                    /*
                     If end then NAK the byte and put Stop. Else continue
                     with ACK of the current byte and receive the next byte
                    */
                    if(g_I2CRepeatedStart)
                    {
                        /*
                         Send the Slave Address with RnW as Receive. If only byte is
                         to be received then send START and STOP else send START
                        */
                        I2CMasterSlaveAddrSet(I2C0_BASE, 0x7d, pdTRUE);
                        if(g_ui8MasterBytesLength == 1)
                        {
                            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
                        }
                        else
                        {
                            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
                        }

                        /*
                         Change the Repeated Start Flag to false as the condition
                         is now to receive data
                        */
                        g_I2CRepeatedStart = 0;
                    }
                    else if(g_ui8MasterBytes == (g_ui8MasterBytesLength - 2))
                    {
                        /*
                         Read the byte from I2C Buffer and decrement the number
                         of bytes counter to see if end has been reached or not
                        */
                        g_ui8MasterRxData[g_ui8MasterBytes++] = I2CMasterDataGet(I2C0_BASE);

                        /*
                         Put a STOP Condition on the bus
                        */
                        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
                    }
                    else
                    {
                        /*
                         Read the byte from I2C Buffer and decrement the number
                         of bytes counter to see if end has been reached or not
                        */
                        g_ui8MasterRxData[g_ui8MasterBytes++] = I2CMasterDataGet(I2C0_BASE);

                        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
                    }
                }

                break;



            case BNO_CALIB:
                /*
                 Move the current state to the previous state
                 Else continue with the transmission till last byte
                */
                g_PrevState = g_CurrState;

                break;


            case ERROR:
                g_CurrState = I2C_ERR_STATE;
                break;


            default:
                g_CurrState = I2C_ERR_STATE;
                break;

        }
    }

}

void BNO_init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), pdTRUE);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE,GPIO_PIN_3);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), pdTRUE);
    I2CMasterIntClear(I2C0_BASE,pdTRUE);
    I2CMasterIntEnableEx(I2C0_BASE , I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK
                         | I2C_MASTER_INT_DATA);
    IntEnable(INT_I2C0);
    g_CurrState=I2C_OP_IDLE;
}

uint8_t BNO_WriteRegister(uint8_t reg8bits, uint8_t dataWriting)
{

}

uint8_t BNO_ReadRegister(uint8_t reg8bits)
{

}


//Interrupt handler code gotten from: https://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/t/470710
//and modified by me
void BNO_IntHandler()
{
    uint32_t I2CMasterInterruptStatus;

    I2CMasterInterruptStatus = I2CMasterIntStatusEx(I2C0_BASE, pdTRUE);
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken=pdFALSE;
    xResult = xEventGroupSetBitsFromISR(
                              Serials,   /* The event group being updated. */
                              I2CMasterInterruptStatus, /* The bits being set. */
                              &xHigherPriorityTaskWoken );
    if(xResult != pdFAIL)
    {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    I2CMasterIntClearEx(I2C0_BASE, I2CMasterInterruptStatus);


}
