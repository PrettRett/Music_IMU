/**
 * Creado por; Daniel Moreno de Jesús, en el día 09/03/2018
 *
 * Main del proyecto Music_IMU
 */
#include "FreeRTOS.h"
#include "queue.h"
#include "list.h"
#include "event_groups.h"
#include "portmacro.h"
#include "task.h"
#include "FreeRTOSconfig.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "stdbool.h"
#include "stdint.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "BLE_serial.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    while(1);
}


void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    while(1);
}

void vApplicationMallocFailedHook( void )
{
    while(1);
}

void vApplicationIdleHook( void )
{
    SysCtlSleep();
}

void main()
{


    //
    // Set the clocking to run at 40 MHz from the PLL.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);    //Ponermos el reloj principal a 40 MHz (200 Mhz del Pll dividido por 5)


    // Get the system clock speed.
    uint32_t g_ui32SystemClock = SysCtlClockGet();


    //Habilita el clock gating de los perifericos durante el bajo consumo --> Hay que decirle los perifericos que queramos que sigan andando usando la funcion SysCtlSleepEnable(...) en cada uno de ellos
    SysCtlPeripheralClockGating(true);


    //
    // Inicializa la UARTy la configura a 115.200 bps, 8-N-1 .
    //se usa para mandar y recibir mensajes y comandos por el puerto serie
    // Mediante un programa terminal como gtkterm, putty, cutecom, etc...
    //
    UARTBLEinit();

    IntMasterEnable();

    if(xTaskCreate(BLE_serialTask, "BLE_serial", 128, 0, tskIDLE_PRIORITY+1, 0)!=pdPASS)
        while(1);

    vTaskStartScheduler();
    while(1);
}
