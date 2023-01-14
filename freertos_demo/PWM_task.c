#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define PWMTASKSTACKSIZE       500        // Stack size in words

extern xQueueHandle g_pLEDQueue;

uint32_t temp_max,temp_min,temp,vel;
bool bstart,btemp;
xQueueHandle g_pTempQueue;
//*****************************************************************************
//
// A tarefa recebe os valores de temperatura, calcula a velocidade e manda
// um sinal PWM para a ventoinha
//
//*****************************************************************************
static void
PWMTask(void *pvParameters)
{
    portBASE_TYPE xStatus;
    while(1)
    {
        while (bstart==1)
        {
            xStatus = xQueueReceive( g_pTempQueue, &temp, portMAX_DELAY );
            if( xStatus == pdPASS )
            {
                if (temp>=temp_max)
                {
                    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
                }
                else{
                    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
                    }
                if(temp>temp_min)
                {
                    if(btemp == false)
                    {
                        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000);
                        PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); //inicializacvao do motor para ultrapassar a velocidade minima
                        //delay
                        btemp = true;
                    }
                    if(temp < temp_max)
                        {
                            vel = (temp-temp_min)/(temp_max-temp_min)*30000;
                            if(vel>(30000*0.2))
                            {
                             PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,vel);
                            }
                            else
                            {
                                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000*0.2);
                            }
                        }
                    if(temp >= temp_max)
                    {
                        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000);
                    }
                }
                else
                {
                    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true);
                    btemp = false;

                }
            }
        }
    }
}

//*****************************************************************************
//
// Initializes the PWM task.
//
//*****************************************************************************
uint32_t
PWMTaskInit(void)
{
     SysCtlPWMClockSet(SYSCTL_PWMDIV_1); // sem pre scaler

     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

     SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

     GPIOPinConfigure(GPIO_PB4_M0PWM2);
     GPIOPinConfigure(GPIO_PB5_M0PWM3);

     GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_5);
     GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);

     PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

     //Set the Period (expressed in clock ticks)
     PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 30000);

     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,15000);//pwm do buzzer
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,0);

     PWMGenEnable(PWM0_BASE, PWM_GEN_1);




     PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
     PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, false);

    //
    // Create the PWM task.
    //
    if(xTaskCreate(PWMTask, (const portCHAR *)"PWM",
                   PWMTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_PWM_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
