/*
 * BNO055.c
 *
 *  Created on: 27 mar. 2018
 *      Author: Daniel
 */

#include "BNO055.h"

void BNO_COMM(void *pvParameters)
{

}

void BNO_init()
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), pdTRUE);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE,GPIO_PIN_3);
    I2CMasterIntEnableEx(I2C2_BASE,
                         (I2C_MASTER_INT_STOP| I2C_MASTER_INT_NACK |
                           I2C_MASTER_INT_DATA));
}

uint8_t BNO_WriteRegister(uint8_t reg8bits, uint8_t dataWriting)
{

}

uint8_t BNO_ReadRegister(uint8_t reg8bits)
{

}

void BNO_IntHandler()
{

}
