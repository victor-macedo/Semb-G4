#include <I2C_task.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LCD.h"

xQueueHandle g_pKEYQueue;
xQueueHandle g_pTempQueue;

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run at 50 MHz from the PLL.
    //


    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    //
    // Cria as Queue do sistema
    //
    g_pKEYQueue = xQueueCreate(48, sizeof(char));
    g_pTempQueue = xQueueCreate(32, sizeof(float));
    //
    // Create the I2C task.
    //
    if(I2CTaskInit() != 0)
    {

        while(1)
        {
        }
    }

    //
    // Create the Key task.
    //
    if(KEYTaskInit() != 0)
    {

        while(1)
        {
        }
    }
    //
    // Create the LCD task.
    //
    if(LCDTaskInit() != 0)
        {

            while(1)
            {
            }
        }
    //
    // Create the PWM task.
    //
    if(PWMTaskInit() != 0)
        {

            while(1)
            {
            }
        }

    //
    // Create the Att task.
    //
    if(AttTaskInit() != 0)
        {

            while(1)
            {
            }
        }
    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //

    while(1)
    {
    }
}
