/*
 * BNO055.c
 *
 *  Created on: 27 mar. 2018
 *      Author: Daniel
 */

#include "BNO055.h"

void BNO_COMM(void *pvParameters)
{
    int
    while(1)
    {
        //posiblemente no haga falta usar la linea de abajo comentada
        //EventBits_t aux=xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        switch (g_CurrState)
        {
            case BNO_INIT:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_CONF;

                int reg=0;
                vTaskDelay(configTICK_RATE_HZ*0.4);
                while(BNO_ReadRegister(BNO055_CHIP_ID_ADDR,&reg,1)<1);
                if(dir==0xA0)
                {
                    BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR,0x20);//hacemos un reset al modulo
                    vTaskDelay(configTICK_RATE_HZ*0.7);
                    do
                    {
                        BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1);
                    }
                    while(reg!=0x00);
                    BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                    mode=OPERATION_MODE_CONFIG;
                }
                else
                    g_CurrState = ERROR;

            break;


            case BNO_CONF:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_RDY;

                int reg=0;
                if(mode!=OPERATION_MODE_CONFIG)
                {
                    BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                    mode=OPERATION_MODE_CONFIG;
                    vTaskDelay(configTICK_RATE_HZ*0.1);
                }

                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00);
                BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR, 0x80);
                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x01);
                vTaskDelay(configTICK_RATE_HZ*0.05);
                BNO_WriteRegister(ACC_Config,ACC_PARAM);
                if(BNO_ReadRegister(ACC_Config,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                    break;
                }
                if(reg!=ACC_PARAM)
                {
                    g_CurrState=ERROR;
                    break;
                }

                BNO_WriteRegister(MAG_Config,MAG_PARAM);
                if(BNO_ReadRegister(MAG_Config,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                    break;
                }
                if(reg!=MAG_PARAM)
                {
                    g_CurrState=ERROR;
                    break;
                }

                BNO_WriteRegister(GYR_Config_0, GYR_PARAM_0);
                if(BNO_ReadRegister(GYR_Config_0,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                    break;
                }
                if(reg!=GYR_PARAM_0)
                {
                    g_CurrState=ERROR;
                    break;
                }

                BNO_WriteRegister(GYR_Config_1, GYR_PARAM_0);
                if(BNO_ReadRegister(GYR_Config_1,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                    break;
                }
                if(reg!=GYR_PARAM_1)
                {
                    g_CurrState=ERROR;
                    break;
                }

                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00);
                vTaskDelay(configTICK_RATE_HZ*0.05);

                BNO_WriteRegister(BNO055_UNIT_SEL_ADDR, ((0x0) /* m/s^2 */ | ( 0x1 << 1 )/* Radians per second */
                                                         | (0x1 << 2)/* Vector de euler en radianes */ | (0x0 << 4 )/* grados centígrados para el sensor de temp */));

            break;


            case BNO_RDY:

            break;


            case BNO_READ:
                break;



            case BNO_CALIB:
                g_PrevState = g_CurrState;

                break;


            case ERROR:
                g_CurrState = ERROR;
                break;


            default:
                g_CurrState = ERROR;
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
    I2CMasterEnable(I2C0_BASE);
    g_CurrState=BNO_INIT;
}

uint8_t BNO_WriteRegister(uint8_t reg8bits, uint8_t dataWriting)
{
    I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
    I2CMasterDataPut(I2C0_BASE, reg8bits);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    do
    {
        I2CMasterDataPut(I2C0_BASE, dataWriting);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    }
    while(!((xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY))&(ACK_FLAG|DATA_FLAG)));
    return pdTRUE;
}

int8_t BNO_ReadRegister(uint8_t firstRegToRead, uint8_t *bytesReadBuff, uint8_t bytesToRead)
{
    uint8_t count=0;
    do
    {
        I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
        I2CMasterDataPut(I2C0_BASE, reg8bits);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    }
    while(!((xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY))&(ACK_FLAG|DATA_FLAG)));

    do
    {
        I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdTRUE);
        if(bytesToRead>1)
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        else
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    while(!((xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY))&(ACK_FLAG|DATA_FLAG)));

    for(count=0,count<bytesToRead,count++)
    {
        bytesReadBuff[count]=I2CMasterDataGet(I2C0_BASE);
        if((count+1)<bytesToRead)
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        if(xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY)&(STOP_FLAG|NACK_FLAG))
            return -1;
    }
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    return count;
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
