/*
 * BNO055.c
 *
 *  Created on: 27 mar. 2018
 *      Author: Daniel
 */

#include "BNO055.h"

int8_t BNO_WriteRegister(uint8_t reg8bits, uint8_t dataWriting)
{

    I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
    I2CMasterDataPut(I2C0_BASE, reg8bits);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    if(!((xEventGroupWaitBits(Signals_BNO, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)))
        return -1;

    I2CMasterDataPut(I2C0_BASE, dataWriting);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    if(!((xEventGroupWaitBits(Signals_BNO, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)))
        return -1;
    return 0;
}

int8_t BNO_ReadRegister(uint8_t firstRegToRead, uint8_t *bytesReadBuff, uint8_t bytesToRead)
{
    uint8_t count=0;
    I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdFALSE);
    I2CMasterDataPut(I2C0_BASE, firstRegToRead);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    if(!((xEventGroupWaitBits(Signals_BNO, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)))
        return -1;
    while(I2CMasterBusy(I2C0_BASE));

    I2CMasterSlaveAddrSet(I2C0_BASE, BNO_ADDRESS, pdTRUE);
    if(bytesToRead>1)
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    else
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    if(!((xEventGroupWaitBits(Signals_BNO, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1))&(ACK_DATA_FLAG)))
        return -1;
    while(I2CMasterBusy(I2C0_BASE));

    for(count=0;count<(bytesToRead-1);count++)
    {
        bytesReadBuff[count]=(0xFF & I2CMasterDataGet(I2C0_BASE));
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        if(xEventGroupWaitBits(Signals_BNO, NACK_FLAG|STOP_FLAG|ACK_DATA_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1)&(STOP_FLAG|NACK_FLAG))
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

void BNO_Reset(void)
{
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x00);
    vTaskDelay(configTICK_RATE_HZ*0.1);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x10);
    vTaskDelay(configTICK_RATE_HZ*0.65);
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

                BNO_Reset();
                if(BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                if(BNO_ReadRegister(BNO055_CHIP_ID_ADDR,&reg,1))
                {
                    g_CurrState = ERROR;
                    break;
                }
                if(reg==0xA0)
                {
                    if(BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR,0x20)<0)//hacemos un reset al modulo
                    {
                        g_CurrState = ERROR;
                        break;
                    }
                    do
                    {
                        BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1);
                    }
                    while(reg!=0x00);
                    if(BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG))
                    {
                        g_CurrState = ERROR;
                        break;
                    }
                    mode_BNO=OPERATION_MODE_CONFIG;
                    if(BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1)<0)
                    {
                        g_CurrState = ERROR;
                        break;
                    }
                    if(BNO_WriteRegister(BNO055_PWR_MODE_ADDR,POWER_MODE_NORMAL)<0)
                    {
                        g_CurrState = ERROR;
                        break;
                    }
                }
                else
                    g_CurrState = ERROR;

            break;


            case BNO_CONF:
                g_PrevState = g_CurrState;
                g_CurrState = BNO_RDY;

                if(BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                vTaskDelay(configTICK_RATE_HZ*0.252);
                if(BNO_ReadRegister(BNO055_PAGE_ID_ADDR,&reg,1)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                if(mode_BNO!=OPERATION_MODE_CONFIG)
                {
                    BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG);
                    mode_BNO=OPERATION_MODE_CONFIG;
                }
                if(BNO_WriteRegister(BNO055_SYS_TRIGGER_ADDR, 0x80)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                if(BNO_ReadRegister(BNO055_SYS_TRIGGER_ADDR,&reg,1)<0)
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

                if(BNO_WriteRegister(BNO055_PAGE_ID_ADDR, 0x00)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                vTaskDelay(configTICK_RATE_HZ*0.1);

                if(BNO_WriteRegister(BNO055_OPR_MODE_ADDR,OPERATION_MODE_CONFIG)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                mode_BNO=OPERATION_MODE_CONFIG;
                if(BNO_WriteRegister(BNO055_UNIT_SEL_ADDR, UNIT_PARAM )<0)
                    g_CurrState = ERROR;
            break;


            case BNO_RDY:
                g_PrevState = g_CurrState;
                mode_BNO=OPERATION_MODE_NDOF;
                if(BNO_WriteRegister(BNO055_OPR_MODE_ADDR,mode_BNO)<0)
                {
                    g_CurrState = ERROR;
                    break;
                }
                EventBits_t aux=xEventGroupWaitBits(Signals_BNO, READ_FLAG|CALIB_FLAG, pdTRUE, pdFALSE, portMAX_DELAY);
                if(aux&READ_FLAG)
                {
                    g_CurrState=BNO_READ;
                    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()-1);//cargamos una cuenta de 1 segundo
                    t_extra=0;
                    TimerEnable(TIMER0_BASE, TIMER_A);
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x00);
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

                if(xEventGroupWaitBits(Signals_BNO, SENT_FAIL_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0)&SENT_FAIL_FLAG)
                {
                    t_extra=t_extra+read_time;
                }
                else
                {
                    t_extra=0;
                }
                read_time=SysCtlClockGet()-TimerValueGet(TIMER0_BASE, TIMER_A)+t_extra;
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
                xEventGroupSetBits(Signals_Comm,DATA_SEND_FLAG);
                uint8_t prev_mode=mode_BNO;
                BNO_ReadRegister(BNO055_OPR_MODE_ADDR, &mode_BNO,1);
                if(mode_BNO!=prev_mode || g_CurrState==ERROR)
                {
                    xEventGroupSetBits(Signals_Comm,TRANS_END_FLAG);
                    g_CurrState=ERROR;
                    break;
                }

                /* Código para parar la lectura y pasar a modo RDY */
                EventBits_t event = xEventGroupWaitBits(Signals_BNO, END_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.01);
                if(event&READ_FLAG)
                {
                    xEventGroupSetBits(Signals_Comm,TRANS_END_FLAG);
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
                while((reg!=0xFF)&&(xEventGroupWaitBits(Signals_BNO, CALIB_FLAG, pdTRUE, pdFALSE, configTICK_RATE_HZ*0.1)==0));
                if(reg==0xFF)
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x0E);
                else
                    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0x00);
                xEventGroupSetBits(Signals_Comm,CALIB_END_FLAG);

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


    //--------------------Habilitar el ENABLE del HM-10-------------------
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0x00);
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
                              Signals_BNO,   /* The event group being updated. */
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
