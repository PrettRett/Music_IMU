/*
 * BNO055.c
 *
 *  Created on: 27 mar. 2018
 *      Author: Daniel
 */

#include "BNO055.h"

uint8_t BNO_WriteRegister(uint8_t reg8bits, uint8_t dataWriting)
{
    do
    {
        do
        {
            I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
            I2CMasterDataPut(I2C0_BASE, reg8bits);
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        }
        while(!((xEventGroupWaitBits(Signals, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)));

        I2CMasterDataPut(I2C0_BASE, dataWriting);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    }
    while(!((xEventGroupWaitBits(Signals, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)));
    return pdTRUE;
}

int8_t BNO_ReadRegister(uint8_t firstRegToRead, uint8_t *bytesReadBuff, uint8_t bytesToRead)
{
    uint8_t count=0;
    do
    {
        I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
        I2CMasterDataPut(I2C0_BASE, firstRegToRead);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    }
    while(!((xEventGroupWaitBits(Signals, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)));
    while(I2CMasterBusy(I2C0_BASE));

    do
    {
        I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdTRUE);
        if(bytesToRead>1)
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        else
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    while(!((xEventGroupWaitBits(Signals, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)));
    while(I2CMasterBusy(I2C0_BASE));

    for(count=0;count<(bytesToRead-1);count++)
    {
        bytesReadBuff[count]=(0xFF & I2CMasterDataGet(I2C0_BASE));
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        if(xEventGroupWaitBits(Signals, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1)&(STOP_FLAG|NACK_FLAG))
                return -1;
        while(I2CMasterBusy(I2C0_BASE));
    }
    bytesReadBuff[count]=(0xFF & I2CMasterDataGet(I2C0_BASE));
    if(bytesToRead>1)
    {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        while(I2CMasterBusy(I2C0_BASE));
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
    }
    else
    {
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    }


    return count;
}

void BNO_COMM(void *pvParameters)
{
    g_CurrState=BNO_INIT;
    uint8_t reg=0;
    while(1)
    {
        //posiblemente no haga falta usar la linea de abajo comentada
        //EventBits_t aux=xEventGroupWaitBits(Serials, ACK_FLAG|NACK_FLAG|STOP_FLAG|DATA_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
        switch (g_CurrState)
        {
            case BNO_INIT:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_CONF;

                vTaskDelay(configTICK_RATE_HZ*0.4);
                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00);
                BNO_ReadRegister(BNO055_CHIP_ID_ADDR,&reg,1);
                if(reg==0xA0)
                {
                    BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR,0x20);//hacemos un reset al modulo
                    do
                    {
                        BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1);
                    }
                    while(reg!=0x00);
                    BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                    mode_BNO=OPERATION_MODE_CONFIG;
                    BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1);
                    BNO_WriteRegister(BNO055_PWR_MODE_ADDR,POWER_MODE_NORMAL);
                }
                else
                    g_CurrState = ERROR;

            break;


            case BNO_CONF:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_RDY;

                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00);
                vTaskDelay(configTICK_RATE_HZ*0.252);
                BNO_ReadRegister(BNO055_PAGE_ID_ADDR,&reg,1);
                if(mode_BNO!=OPERATION_MODE_CONFIG)
                {
                    BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                    mode_BNO=OPERATION_MODE_CONFIG;
                }
                BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR, 0x80);
                BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1);
                /*mode_BNO=OPERATION_MODE_AMG;
                BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_AMG);
                BNO_ReadRegister(BNO055_OPR_MODE_ADDR,&reg,1);
                vTaskDelay(configTICK_RATE_HZ*0.252);
                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x01);
                vTaskDelay(configTICK_RATE_HZ*0.252);
                BNO_WriteRegister(ACC_Config,ACC_PARAM);
                BNO_WriteRegister(MAG_Config,MAG_PARAM);
                BNO_WriteRegister(GYR_Config_0, GYR_PARAM_0);
                BNO_WriteRegister(GYR_Config_1, GYR_PARAM_0);
                BNO_ReadRegister(BNO055_PAGE_ID_ADDR,&reg,1);
                if(reg!=0x01)
                {
                    g_CurrState=ERROR;
                }
                if(BNO_ReadRegister(ACC_Config,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                    break;
                }
                if(reg!=ACC_PARAM)
                {
                    g_CurrState=ERROR;
                }
                if(BNO_ReadRegister(MAG_Config,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                }
                if(reg!=MAG_PARAM)
                {
                    g_CurrState=ERROR;
                }
                if(BNO_ReadRegister(GYR_Config_0,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                }
                if(reg!=GYR_PARAM_0)
                {
                    g_CurrState=ERROR;
                }
                if(BNO_ReadRegister(GYR_Config_1,&reg,1)<0)
                {
                    g_CurrState=ERROR;
                }
                if(reg!=GYR_PARAM_1)
                {
                    g_CurrState=ERROR;
                }*/

                BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00);
                vTaskDelay(configTICK_RATE_HZ*0.1);

                BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                mode_BNO=OPERATION_MODE_CONFIG;
                BNO_WriteRegister(BNO055_UNIT_SEL_ADDR, UNIT_PARAM );

            break;


            case BNO_RDY:
                g_PrevState = g_CurrState;
                mode_BNO=OPERATION_MODE_NDOF;
                BNO_WriteRegister(BNO055_OPR_MODE_ADDR,mode_BNO);
                EventBits_t aux=xEventGroupWaitBits(Signals, READ_FLAG|CALIB_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
                if(aux&READ_FLAG)
                {
                    g_CurrState=BNO_READ;
                    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()-1);//cargamos una cuenta de 1 segundo
                    TimerEnable(TIMER0_BASE, TIMER_A);
                }
                else if(aux&CALIB_FLAG)
                    g_CurrState=BNO_CALIB;
                /* Código para pasar a modo lectura */


                /* Código para pasar a modo configurar */

                /* Código para calibrar */

            break;


            case BNO_READ:
                g_PrevState = g_CurrState;
                /* Código para leer todos los registros de salida del BNO */
                if(BNO_ReadRegister(BNO055_QUATERNION_DATA_W_LSB_ADDR,part_read+24,20)<0)
                    g_CurrState = ERROR;

                vTaskDelay(configTICK_RATE_HZ*0.006);

                // Hacemos ahora la lectura del timer, y reiniciamos la cuenta para saber cuanto
                // es el tiempo entre que se capturan las lecturas de media.
                read_time=SysCtlClockGet()-TimerValueGet(TIMER0_BASE, TIMER_A);
                TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()-1);

                if(BNO_ReadRegister(BNO055_QUATERNION_DATA_W_LSB_ADDR,mean_read+24,20)<0)
                    g_CurrState = ERROR;
                int16_t *n1=(int16_t*)part_read;
                int16_t *n2=(int16_t*)mean_read;
                int i;
                for(i=0;i<23;i++)
                {
                    n2[i]=(n2[i]+n1[i])/2;
                }
                xSemaphoreTake(mut,portMAX_DELAY);
                memcpy(sensors_value.mult_read,mean_read,sizeof(mean_read));
                //---------Código previo(poco optimizado)--------------
/*                int i, s;
                s=pdTRUE;
                for(i=0; i<45; i++)
                {
                    s=s && xQueueSend(xCharsForTx0,&mult_read[i++],portMAX_DELAY);
                    s=s && xQueueSend(xCharsForTx0,&mult_read[i],portMAX_DELAY);
                    s=s && xQueueSend(xCharsForTx0,&separation,portMAX_DELAY);

                }
                s=s && xQueueSend(xCharsForTx0,&end[0],portMAX_DELAY);
                s=s && xQueueSend(xCharsForTx0,&end[1],portMAX_DELAY);
                if(s!=pdTRUE)
                    g_CurrState = ERROR;*/
                xSemaphoreGive(mut);
                xEventGroupSetBits(Signals,DATA_SEND_FLAG);
                uint8_t prev_mode=mode_BNO;
                BNO_ReadRegister(BNO055_OPR_MODE_ADDR, &mode_BNO,1);
                if(mode_BNO!=prev_mode || g_CurrState==ERROR)
                {
                    g_CurrState=ERROR;
                    break;
                }

                /* Código para parar la lectura y pasar a modo RDY */
                if(xEventGroupWaitBits(Signals, READ_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.01)&READ_FLAG)
                {
                    g_CurrState = BNO_RDY;
                    TimerDisable(TIMER0_BASE, TIMER_A);
                }

                break;



            case BNO_CALIB:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_RDY;

                GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x02);
                do{
                    BNO_ReadRegister(BNO055_CALIB_STAT_ADDR,&reg,1);
                }
                while((reg<=0x1F)&&(xEventGroupWaitBits(Signals, CALIB_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1)==0));
                if(reg>=0x1F)
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x0E);
                else
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x0E);

                break;


            case ERROR:
                TimerDisable(TIMER0_BASE, TIMER_A);
                if(g_PrevState==BNO_RDY || g_PrevState==BNO_READ || g_PrevState==BNO_CALIB)
                    g_CurrState=BNO_CONF;
                else
                    g_CurrState=BNO_INIT;
                g_PrevState=ERROR;
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
    GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_3);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE,GPIO_PIN_2);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), pdTRUE);
    I2CMasterGlitchFilterConfigSet(I2C0_BASE,I2C_MASTER_GLITCH_FILTER_2);
    I2CMasterIntClear(I2C0_BASE);
    I2CMasterIntEnable(I2C0_BASE);
    I2CMasterIntEnableEx(I2C0_BASE , I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK
                         | I2C_MASTER_INT_DATA);
    IntEnable(INT_I2C0);
    I2CMasterEnable(I2C0_BASE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x00);

    //----------------------Timer para enviar el tiempo entre las lectura-----------------------

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);
    TimerClockSourceSet(TIMER0_BASE,TIMER_CLOCK_SYSTEM);
    // Configura el Timer0 para cuenta periodica de 32 bits (no lo separa en TIMER0A y TIMER0B)
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Habilita interrupcion del modulo TIMER
    // usaremos la interrupción para decicir que el
    // módulo se ha quedado colgado
    IntEnable(INT_TIMER0A);
    // Y habilita, dentro del modulo TIMER0, la interrupcion de particular de "fin de cuenta"
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


//Interrupt handler code gotten from: https://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/t/470710
//and modified by me
void BNO_IntHandler()
{
    uint32_t I2CMasterInterruptStatus;

    I2CMasterInterruptStatus = I2CMasterIntStatusEx(I2C0_BASE, pdTRUE);
    I2CMasterIntClearEx(I2C0_BASE, I2CMasterInterruptStatus);
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken=pdFALSE;
    if(I2CMasterInterruptStatus&I2C_MASTER_INT_DATA)
        I2CMasterInterruptStatus=ACK_DATA_FLAG;
    else
        I2CMasterInterruptStatus>>=2;
    xResult = xEventGroupSetBitsFromISR(
                                Signals,   /* The event group being updated. */
                              I2CMasterInterruptStatus, /* The bits being set. */
                              &xHigherPriorityTaskWoken );
    if(xResult != pdFAIL)
    {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }


}

void Timer0AHandler()
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    g_CurrState=ERROR;
    TimerDisable(TIMER0_BASE, TIMER_A);
}
