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
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "task.h"
#include "stdbool.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "FreeRTOSconfig.h"
#include "BLE_serial.h"

EventGroupHandle_t Serials;

int main(void)
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    UARTEnable(UART0_BASE);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Esta funcion habilita la interrupcion de la UART y le da la prioridad adecuada si esta activado el soporte para FreeRTOS
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);   //La UART tiene que seguir funcionando aunque el micro esta dormido
    UARTClockSourceSet(UART0_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),9600,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);


    //Inicializar UART1 --------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    UARTEnable(UART1_BASE);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART1);   //La UART tiene que seguir funcionando aunque el micro esta dormido;
    UARTClockSourceSet(UART1_BASE,UART_CLOCK_SYSTEM);
    UARTConfigSetExpClk(UART1_BASE,SysCtlClockGet(),115200,
                       UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|
                       UART_CONFIG_PAR_NONE);



    //Inicializa el puerto F (LEDs) como GPIO
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0); //LEDS APAGADOS

    //Event_groups para avisar a las funciones de los serial
    Serials=xEventGroupCreate();



    vTaskStartScheduler();
    while(1);
}
